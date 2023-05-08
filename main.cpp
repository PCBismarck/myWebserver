#include <concepts>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
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
        auto resp = conn->getResponse();
        resp->setStatusCode(HTTPBase::StatusCode::NOT_FOUND);
        resp->setDescription("404 Not Found");
        std::string body = "404 Not Found\n";
        resp->setBody(body);
        resp->addHeader("Content-Type", "text/plain");
        resp->addHeader("Content-Length", std::to_string(body.size()));
        conn->respReady();
    });

    router->Get("/hello", [](HTTPConn* conn) {
        LOG_DEBUG("client[%d] Handler GET /hello was called", conn->getFd());
        auto resp = conn->getResponse();
        resp->setVersion(1, 1);
        resp->setStatusCode(HTTPBase::StatusCode::OK);
        resp->setDescription("OK");
        std::string query = conn->getRequest()->query("name");
        LOG_INFO("query: %s", query.c_str());
        std::string body = query + " Hello World!\n";
        resp->setBody(body);
        resp->addHeader("Content-Type", "text/plain");
        resp->addHeader("Content-Length", std::to_string(body.size()));
        conn->respReady();
    });
    w.init(std::move(router), 9123, 3, 8, true, LogLevel::INFO);
    w.run();
}
