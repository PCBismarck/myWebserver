#ifndef __HTTPCONN_H__
#define __HTTPCONN_H__

#include <memory>
#include <netinet/in.h>
#include <unistd.h>
// #include "../log/log.h"
#include "httprequest.h"
#include "httpresponse.h"

// TODO 根据设置情况发送响应报文
class HTTPConn
{
    using Request = std::shared_ptr<HTTPRequest>;
    using Response = std::shared_ptr<HTTPResponse>;

public:
    HTTPConn()
    {
        is_close = true;
        is_resp_setted = false;
    }

    ~HTTPConn() { closeConn(); }

    void init(int socket_fd, const sockaddr_in& addr)
    {
        m_socket_fd = socket_fd;
        m_addr = addr;
        is_close = false;
        is_resp_setted = false;
    }

    int getFd() const { return m_socket_fd; }

    void setRequest(Request& req) { m_req = std::move(req); }

    void setResponse(Response& resp)
    {
        m_resp = std::move(resp);
        is_resp_setted = true;
    }

    Request getRequest() const { return m_req; }

    Response getResponse() const { return m_resp; }

    void closeConn()
    {
        if (is_close)
            return;
        m_req.reset();
        m_resp.reset();
        is_close = true;
        is_resp_setted = false;
        close(m_socket_fd);
        // LOG_INFO("close fd %d", m_socket_fd);
    }

    bool isClose() const { return is_close; }
    bool isRespSetted() const { return is_resp_setted; }

private:
    bool is_close;
    bool is_resp_setted;

    int m_socket_fd;
    struct sockaddr_in m_addr;

    Request m_req;
    Response m_resp;
};

#endif   // __HTTPCONN_H__