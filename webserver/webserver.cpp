#include "webserver.h"
#include <atomic>
#include <bits/types/struct_iovec.h>
#include <bits/types/time_t.h>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include "../log/log.h"

std::atomic<int> WebServer::user_count = 0;

// 初始化部分
WebServer::WebServer()
{
    m_epoll = std::make_unique<Epoll>();
    m_timers = std::make_unique<TimerList>(60000);
}

WebServer::~WebServer()
{
    close(m_listenfd);
    is_stop = true;
    m_sfd2conn.clear();
}

void WebServer::init(std::unique_ptr<Router> router, int port, int timeout,
                     int threadNum, bool openlog, LogLevel logLevel,
                     int log_que_size)
{
    user_count = 0;
    m_port = port;
    m_timeout = timeout;
    m_threadNum = threadNum;
    m_logLevel = logLevel;

    m_workers = std::make_unique<Threadpool>(m_threadNum);

    // 设置超时计算函数
    m_timers->enableAutoExpire([timeslot = m_timeout] {
        time_t now = time(nullptr) + timeslot;
        // LOG_INFO("cur time: %d, set timeout: %d", time(nullptr), now);
        return now;
    });

    m_router = std::move(router);
    initSocket();                                  // 初始化socket
    initLogger(openlog, logLevel, log_que_size);   // 初始化日志
}

void WebServer::initSocket()
{
    // 设置监听socket
    // TCP 非阻塞
    m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
    setNonBlocking(m_listenfd);
    assert(m_listenfd >= 0);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;   // IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    int ret =
        bind(m_listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    assert(ret >= 0);

    ret = listen(m_listenfd, 10);
    assert(ret >= 0);
    m_epoll->addFd(m_listenfd, EPOLLIN | listen_event);
}

void WebServer::initLogger(bool openlog, LogLevel logLevel, int log_que_size)
{
    if (!openlog)
        return;
    Logger::getInstance()->init(logLevel, log_que_size, ".log", "./logfile");
    if (is_stop) {
        LOG_ERROR("========== Server init error!==========");
    } else {
        LOG_DEBUG("========== Server init ==========");
        LOG_DEBUG("Port:%d", m_port);
        LOG_DEBUG("LogSys level: %d", logLevel);
        LOG_DEBUG("Queue size: %d", log_que_size);
    }
}

void WebServer::run()
{
    time_t next_timeout = -1;
    if (!is_stop) {
        LOG_DEBUG("%s", "<================ webserver start ===============>");
    } else {
        LOG_DEBUG("%s", "<=========== webserver start failure ============>");
    }
    while (!is_stop) {
        // 处理超时事件
        if (m_timeout > 0) {
            LOG_INFO("start tick, cur timeout: %d", next_timeout);
            next_timeout = m_timers->tick();
        }
        // 等待事件
        LOG_INFO("epoll wait, timeout: %d", next_timeout);
        int event_count = m_epoll->wait(next_timeout * 1000);
        if (event_count < 0 && errno != EINTR) {
            perror("epoll wait faliure");
            LOG_ERROR("%s", "epoll wait failure");
            break;
        }
        // 处理事件
        for (int i = 0; i < event_count; ++i) {
            int fd = m_epoll->getEventFd(i);
            uint32_t events = m_epoll->getEvents(i);
            LOG_INFO("Event fd: %d, event: %d", fd, events);
            if (fd == m_listenfd) {
                LOG_INFO("fd[%d] listen event", fd);
                acceptConnection();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // 处理异常事件
                LOG_INFO("fd[%d] error event, close connection", fd);
                assert(m_sfd2conn.count(fd) > 0);
                closeConnection(&m_sfd2conn[fd]);
            } else if (events & EPOLLIN) {
                // 处理读事件
                LOG_INFO("fd[%d] read event", fd);
                assert(m_sfd2conn.count(fd) > 0);
                handleRead(&m_sfd2conn[fd]);
            } else if (events & EPOLLOUT) {
                // 处理写事件
                LOG_INFO("fd[%d] write event", fd);
                assert(m_sfd2conn.count(fd) > 0);
                handleWrite(&m_sfd2conn[fd]);
            } else {
                LOG_ERROR("%s", "unexpected event");
            }
        }
    }
}

// LT模式接收新连接
void WebServer::acceptConnection()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int connfd =
        accept(m_listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (connfd < 0) {
        perror("accept connection failure");
        LOG_ERROR("%s", "accept connection failure");
        return;
    }
    if (user_count >= MAX_FD) {
        int cnt = user_count;
        LOG_WARN("too many connections: %d", cnt);
        cnt = send(connfd, "server busy, please contact admin", 34, 0);
        if (cnt < 0) {
            perror("send BUSY response failure");
            LOG_ERROR("send BUSY response failure to client[%d]", connfd);
        }
        close(connfd);
        return;
    }

    ++user_count;
    int cnt = user_count;
    LOG_INFO("client[%d] connected, cur user count: %d", connfd, cnt);
    // 初始化连接
    m_sfd2conn[connfd].init(connfd, client_addr);

    m_epoll->addFd(connfd, conn_event | EPOLLIN);
    LOG_INFO("connfd[%d] add to epoll", connfd);
    setNonBlocking(connfd);

    // 添加超时删除定时器
    m_timers->addTimer(
        new BaseTimer(connfd, std::bind(&WebServer::closeConnection, this,
                                        &m_sfd2conn[connfd])));
}

void WebServer::closeConnection(HTTPConn* conn)
{
    if (conn->isClose()) {
        LOG_DEBUG("client[%d] already closed", conn->getFd());
        return;
    }
    --user_count;
    int cnt = user_count;
    LOG_INFO("cur user count: %d", cnt);
    m_epoll->delFd(conn->getFd());
    conn->closeConn();
    LOG_INFO("client[%d] quit", conn->getFd());
}

int WebServer::readOnce(int connfd, char* buf, int len)
{
    int bytes_read = 0, has_read = 0;
    while (true) {
        bytes_read = read(connfd, buf + has_read, READ_BUFFER_SIZE - has_read);
        if (bytes_read == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            LOG_ERROR("clietn[%d] read error: %s buf: %s", connfd,
                      strerror(errno), buf);
            return -1;
        } else if (bytes_read == 0) {
            LOG_ERROR("clietn[%d] read failure(bytes_read = %d)", connfd,
                      bytes_read);
            return -1;
        }
        LOG_DEBUG("clietn[%d] read bytes = %d", connfd, bytes_read);
        has_read += bytes_read;
    }
    LOG_DEBUG("recv from client[%d]:\n"
              "+++++++++++++++++++++++++++++++++++++++++++++++\n"
              "%s"
              "+++++++++++++++++++++++++++++++++++++++++++++++\n",
              connfd, buf);
    return has_read;
}

void WebServer::handleRequest(HTTPConn* conn)
{
    char buf[READ_BUFFER_SIZE];
    memset(buf, '\0', READ_BUFFER_SIZE);
    int connfd = conn->getFd();
    int bytes_read = 0, has_read = 0;

    LOG_DEBUG("client[%d] handle request", connfd)
    // ET模式，一次性读取完缓存区的数据
    has_read = readOnce(connfd, buf, READ_BUFFER_SIZE);
    if (has_read <= 0)
        return;

    // 设置request
    auto req = conn->getRequest();
    ReqStatus status = req->parse(buf, has_read);
    if (status != ReqStatus::REQ_SUCCESS) {
        LOG_ERROR("parse request failure(status = %d)", status);
        return;
    }
    // conn->setRequest(req);
    LOG_DEBUG("client[%d] setted request", connfd)

    // 根据router获取handler
    std::function<void(HTTPConn*)> task;
    bool ok = m_router->getHandler(req->method(), req->path(), task);
    if (!ok) {
        LOG_DEBUG("get handler failure -> method: %s, path: %s",
                  HTTPBase::method2str(req->method()).c_str(),
                  req->path().c_str());
        return;
    }
    LOG_DEBUG("client[%d] get handler", connfd)
    // 执行handler
    task(conn);
    // 若设置了response，则设置为epollout准备发送response
    if (conn->isRespSetted()) {
        LOG_DEBUG("client[%d] set response", connfd)
        m_epoll->modFd(connfd, conn_event | EPOLLOUT);
    } else {
        m_epoll->modFd(connfd, conn_event | EPOLLIN);
    }
}

int WebServer::addBaseInfo(char* buf, int len, HTTPResponse* resp)
{
    // 添加响应的状态行信息
    int has_write = 0;
    int bytes_write =
        snprintf(buf, WRITE_BUFFER_SIZE - has_write, "HTTP/%s %s\r\n",
                 resp->getVersionStr().c_str(), resp->getStatusStr().c_str());
    has_write += bytes_write;
    // 添加响应的头部信息
    for (auto& [header, content] : resp->getHeaders()) {
        bytes_write = snprintf(buf + has_write, WRITE_BUFFER_SIZE - has_write,
                               "%s: %s\r\n", header.c_str(), content.c_str());
        has_write += bytes_write;
    }
    // 添加响应的空行
    bytes_write =
        snprintf(buf + has_write, WRITE_BUFFER_SIZE - has_write, "\r\n");
    has_write += bytes_write;
    return has_write;
}

char* WebServer::makeBodyInfo(int& len, HTTPResponse* resp)
{
    // 添加响应的body信息
    // 访问文件资源
    if (resp->isAccessResource()) {
        struct stat file_stat;
        if (stat(resp->getFilePath().c_str(), &file_stat) < 0) {
            LOG_DEBUG("stat file[%s] failure", resp->getFilePath().c_str());
            return nullptr;
        }
        if (!(file_stat.st_mode & S_IROTH)) {
            LOG_DEBUG("file[%s] can not be read", resp->getFilePath().c_str());
            return nullptr;
        }
        if (S_ISDIR(file_stat.st_mode)) {
            LOG_DEBUG("file[%s] is a directory", resp->getFilePath().c_str());
            return nullptr;
        }
        int filefd = open(resp->getFilePath().c_str(), O_RDONLY);
        if (filefd < 0) {
            LOG_DEBUG("open file[%s] failure", resp->getFilePath().c_str());
            return nullptr;
        }
        char* file_addr = static_cast<char*>(
            mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, filefd, 0));
        len = file_stat.st_size;
        return file_addr;
    } else {
        len = resp->getBody().size();
        return const_cast<char*>(resp->getBody().c_str());
    }
}

bool WebServer::sendResponse(int connfd, struct iovec* iv, uint32_t total_len)
{
    uint32_t bytes_write = 0;
    LOG_INFO("send response to client[%d], resp len: %d", connfd, total_len);
    while (bytes_write < total_len) {
        int n = writev(connfd, iv, 2);
        LOG_INFO("send response to client[%d], write: %d", connfd, n);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                m_epoll->modFd(connfd, conn_event | EPOLLOUT);
                break;
            }
            LOG_DEBUG("writev failure: %s", strerror(errno));
            return false;
        }
        bytes_write += n;
        if (bytes_write < iv[0].iov_len) {
            iv[0].iov_base = static_cast<char*>(iv[0].iov_base) + n;
            iv[0].iov_len -= n;
        } else {
            iv[1].iov_base = static_cast<char*>(iv[1].iov_base) + n;
            iv[1].iov_len -= n;
        }
    }
    return true;
}

void WebServer::handleResponse(HTTPConn* conn)
{
    char buf[WRITE_BUFFER_SIZE];
    memset(buf, '\0', WRITE_BUFFER_SIZE);
    int connfd = conn->getFd();
    int bytes_write = 0, has_write = 0;
    struct iovec iv[2];
    auto resp = conn->getResponse();

    // 准备响应的body信息
    int body_len = 0;
    char* body_addr = makeBodyInfo(body_len, resp);
    if (body_addr == nullptr) {
        LOG_ERROR("conn[%d] make body info failure", connfd);
        return;
    }
    iv[1].iov_base = body_addr;
    iv[1].iov_len = body_len;
    resp->addHeader("Content-Length", std::to_string(body_len));

    // 准备基本的响应信息
    if (resp->isHeadersSend()) {
        iv[0].iov_base = buf;
        iv[0].iov_len = 0;
    } else {
        has_write = addBaseInfo(buf, WRITE_BUFFER_SIZE, resp);
        if (has_write >= WRITE_BUFFER_SIZE) {
            LOG_ERROR("response header too long(%d bytes)", has_write);
            return;
        }
        iv[0].iov_base = buf;
        iv[0].iov_len = has_write;
    }

    // 将响应的头部信息和body信息一起发送
    sendResponse(connfd, iv, iv[0].iov_len + iv[1].iov_len);
    if (resp->isAccessResource()) {
        munmap((char*)iv[1].iov_base, iv[1].iov_len);
    }
    // 继续监听该连接的读事件
    m_epoll->modFd(connfd, EPOLLIN | conn_event);

    // 关闭连接
    // closeConnection(conn);
}

void WebServer::handleRead(HTTPConn* conn)
{
    LOG_DEBUG("handle read fd(%d)", conn->getFd());
    // refresh timer
    m_timers->refreshTimer(m_timers->getTimer(conn->getFd()));
    m_workers->add(std::bind(&WebServer::handleRequest, this, conn));
}

void WebServer::handleWrite(HTTPConn* conn)
{
    LOG_DEBUG("handle write fd(%d)", conn->getFd());
    // refresh timer
    m_timers->refreshTimer(m_timers->getTimer(conn->getFd()));
    m_workers->add(std::bind(&WebServer::handleResponse, this, conn));
}

void WebServer::setNonBlocking(int fd)
{
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}