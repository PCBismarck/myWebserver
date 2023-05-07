#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <functional>
#include <memory>

#include "../epoll/epoll.h"
#include "../http/httpconn.h"
#include "../http/router.h"
#include "../log/log.h"
#include "../threadpool/threadpool.h"
#include "../timer/timer.h"

const int MAX_FD = 65536;   // 最大文件描述符
const int READ_BUFFER_SIZE = 2048;
const int WRITE_BUFFER_SIZE = 4096;

class WebServer
{
    using Task = std::function<void()>;
    using ConnectionMap = std::unordered_map<int, HTTPConn>;

public:
    WebServer();
    ~WebServer();

    void init(std::unique_ptr<Router> router, int port = 9123, int timeout = 15,
              int threadNum = 8, bool openlog = true,
              LogLevel logLevel = LogLevel::DEBUG, int log_que_size = 1000);
    void run();

    static std::atomic<int> user_count;

private:
    void initSocket();
    void initLogger(bool openlog, LogLevel logLevel, int log_que_size);

    void acceptConnection();
    void closeConnection(HTTPConn* conn);

    void handleRead(HTTPConn* conn);
    void handleWrite(HTTPConn* conn);

    void handleRequest(HTTPConn* conn);
    void handleResponse(HTTPConn* conn);

    int readOnce(int connfd, char* buf, int len);
    int addBaseInfo(char* buf, int len, std::shared_ptr<HTTPResponse> resp);
    char* makeBodyInfo(int& len, std::shared_ptr<HTTPResponse> resp);
    bool sendResponse(int connfd, struct iovec* iv, uint32_t total_len);

    void setNonBlocking(int fd);

private:
    bool is_stop;

    int m_port;
    int m_timeout;   // second
    int m_threadNum;
    int m_listenfd;
    LogLevel m_logLevel;

    uint32_t listen_event = EPOLLRDHUP;
    uint32_t conn_event = EPOLLONESHOT | EPOLLRDHUP;   // EPOLLET |

    std::unique_ptr<Epoll> m_epoll;
    std::unique_ptr<TimerList> m_timers;
    std::unique_ptr<Threadpool> m_workers;
    std::unique_ptr<Router> m_router;
    ConnectionMap m_sfd2conn;
};

#endif   //__WEBSERVER_H__