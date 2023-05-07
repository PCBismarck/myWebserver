#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include "http/httpconn.h"
#include "http/httpinfo.h"
#include "http/httpresponse.h"
#include "http/router.h"
#include "log/log.cpp"
#include "log/log.h"
#include "threadpool/threadpool.cpp"
#include "threadpool/threadpool.h"
#include "webserver/webserver.h"

int main(int, char**)
{
    WebServer w;
    std::unique_ptr<Router> router(std::make_unique<Router>());

    router->Static("/static", "/home/pcbismarck/GitProj/myWebserver/resource");

    router->setDefaultHandler([](HTTPConn* conn) {
        LOG_DEBUG("client[%d] Handler default was called", conn->getFd());
        auto resp = std::make_shared<HTTPResponse>(
            HTTPBase::Version{1, 1}, HTTPBase::StatusCode::NOT_FOUND,
            "Not Found");
        std::string body = "404 Not Found\n";
        resp->setBody(body);
        resp->addHeader("Content-Type", "text/plain");
        resp->addHeader("Content-Length", std::to_string(body.size()));
        conn->setResponse(resp);
    });

    router->Get("/hello", [](HTTPConn* conn) {
        LOG_DEBUG("client[%d] Handler GET /hello was called", conn->getFd());
        auto resp = std::make_shared<HTTPResponse>(
            HTTPBase::Version{1, 1}, HTTPBase::StatusCode::OK, "OK");
        std::string body = "Hello World!\n";
        resp->setBody(body);
        resp->addHeader("Content-Type", "text/plain");
        resp->addHeader("Content-Length", std::to_string(body.size()));
        conn->setResponse(resp);
    });
    w.init(std::move(router), 9123, 3, 16, false, LogLevel::DEBUG);
    w.run();
}
