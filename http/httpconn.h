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
public:
    HTTPConn()
    {
        is_close = true;
        is_resp_setted = false;
        m_req = new HTTPRequest;
        m_resp = new HTTPResponse;
    }

    ~HTTPConn()
    {
        delete m_req;
        delete m_resp;
        closeConn();
    }

    void init(int socket_fd, const sockaddr_in& addr)
    {
        m_socket_fd = socket_fd;
        m_addr = addr;
        is_close = false;
        is_resp_setted = false;
        m_resp->init();
    }

    int getFd() const { return m_socket_fd; }

    // void setRequest(HTTPRequest& req) { m_req = std::move(req); }

    // void setResponse(HTTPResponse& resp)
    // {
    //     m_resp = std::move(resp);
    //     is_resp_setted = true;
    // }

    void respReady() { is_resp_setted = true; }

    HTTPRequest* getRequest() { return m_req; }

    HTTPResponse* getResponse() { return m_resp; }

    void closeConn()
    {
        if (is_close)
            return;
        is_close = true;
        is_resp_setted = false;
        close(m_socket_fd);
    }

    bool isClose() const { return is_close; }
    bool isRespSetted() const { return is_resp_setted; }

private:
    bool is_close;
    bool is_resp_setted;

    int m_socket_fd;
    struct sockaddr_in m_addr;

    HTTPRequest* m_req;
    HTTPResponse* m_resp;
};

#endif   // __HTTPCONN_H__