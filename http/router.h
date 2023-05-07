#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <functional>
#include <regex>
#include <string>
#include <unordered_map>

#include "httpconn.h"
#include "httpinfo.h"

// TODO 资源访问的自动化
class Router
{
    using CallBackFunc = std::function<void(HTTPConn*)>;
    using Route = std::unordered_map<std::string, CallBackFunc>;
    using ResourceMap = std::unordered_map<std::string, std::string>;

public:
    void Get(std::string path, CallBackFunc cb) { m_get[path] = cb; }
    void Post(std::string path, CallBackFunc cb) { m_post[path] = cb; }
    void Put(std::string path, CallBackFunc cb) { m_put[path] = cb; }
    void Delete(std::string path, CallBackFunc cb) { m_delete[path] = cb; }
    void Static(std::string path, std::string resource_dir)
    {
        m_static[path] = resource_dir;
    };

    // 针对静态资源进行处理
    bool getHandler(HTTPBase::Method method, std::string path,
                    CallBackFunc& cb);
    void setDefaultHandler(CallBackFunc cb) { m_default = cb; }

private:
    bool isStaticResource(std::string& path);
    bool getStaticResourceHandler(std::string path, CallBackFunc& cb);
    bool getMethodHandler(HTTPBase::Method method, std::string path,
                          CallBackFunc& cb);

private:
    std::string m_pwd;

    Route m_get;
    Route m_post;
    Route m_put;
    Route m_delete;
    ResourceMap m_static;
    CallBackFunc m_default = nullptr;
};

#endif   // __ROUTER_H__