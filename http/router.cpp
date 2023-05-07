#include "router.h"
#include <memory>
#include "../log/log.h"
#include "httpconn.h"
#include "httpinfo.h"
#include "httpresponse.h"

bool Router::getHandler(HTTPBase::Method method, std::string path,
                        CallBackFunc& cb)
{
    if (isStaticResource(path)) {
        return getStaticResourceHandler(path, cb);
    }
    return getMethodHandler(method, path, cb);
}

bool Router::isStaticResource(std::string& path)
{
    for (auto& [pre, to] : m_static) {
        std::regex reg("^" + pre);
        if (std::regex_search(path, reg)) {
            path = std::regex_replace(path, reg, to);
            return true;
        }
    }
    return false;
}

bool Router::getStaticResourceHandler(std::string path, CallBackFunc& cb)
{
    cb = std::bind(
        [new_path = path](HTTPConn* conn) {
            auto resp = conn->getResponse();
            auto req = conn->getRequest();
            const auto& v = req->version();
            resp->setVersion(v.major, v.minor);
            // resp->setVersion(, int minor)
            resp->setStatusCode(HTTPBase::StatusCode::OK);
            resp->setDescription("OK");
            for (auto& [key, value] : req->headersAll()) {
                resp->addHeader(key, value);
            }
            resp->setKeepAlive(req->isKeepAlive());
            resp->setIsAccessResource(new_path);
            LOG_DEBUG("Get Static resource handler: file[%s]",
                      new_path.c_str());
            conn->respReady();
        },
        std::placeholders::_1);
    return true;
}

bool Router::getMethodHandler(HTTPBase::Method method, std::string path,
                              CallBackFunc& cb)
{
    switch (method) {
    case HTTPBase::Method::GET:
        if (m_get.find(path) == m_get.end())
            break;
        cb = m_get[path];
        return true;
    case HTTPBase::Method::UNKNOWN:
        if (m_get.find(path) == m_get.end())
            break;
        cb = m_get[path];
        return true;
    case HTTPBase::Method::POST:
        if (m_post.find(path) == m_post.end())
            break;
        cb = m_post[path];
        return true;
    case HTTPBase::Method::PUT:
        if (m_put.find(path) == m_put.end())
            break;
        cb = m_put[path];
        return true;
    case HTTPBase::Method::DELETE:
        if (m_delete.find(path) == m_delete.end())
            break;
        cb = m_delete[path];
        return true;
    default:
        break;
    }
    if (m_default) {
        cb = m_default;
        return true;
    }
    return false;
}
