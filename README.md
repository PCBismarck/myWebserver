# myWebserver
- 基于epoll实现的reactor模式的linux服务器，采用非阻塞socket+线程池+事件处理的方式
- 用LRU实现定时器队列，通过回调实现定时器的触发
- 实现了router类，可以通过设置static属性完成url到本地文件夹的映射，快速实现资源访问功能
- 通过在router中进行handler函数的注册，实现业务的平滑拓展，无需关注框架底层专注于业务逻辑
- 基于阻塞队列实现了异步日志系统
- 基于正则表达式进行http协议的解析

快速启动/配置服务器
```
int main(int, char**)
{
    WebServer w;
    std::unique_ptr<Router> router(std::make_unique<Router>());
    
    // 设置静态资源映射，实现资源访问的自动处理
    // 如 /static/log.html 会被映射到主机的 /home/myWebserver/resource/log.html 上 
    router->Static("/static", "/home/myWebserver/resource");

    // 设置默认的HTTP回应方式，处理没有匹配的请求
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
 
    // 为特定的http请求设定对应的回应报文
    router->Get("/hello", [](HTTPConn* conn) {
        LOG_DEBUG("client[%d] Handler GET /hello was called", conn->getFd());
        auto resp = conn->getResponse();
        resp->setVersion(1, 1);
        resp->setStatusCode(HTTPBase::StatusCode::OK);
        resp->setDescription("OK");
        std::string body = "Hello World!\n";
        resp->setBody(body);
        resp->addHeader("Content-Type", "text/plain");
        resp->addHeader("Content-Length", std::to_string(body.size()));
        conn->respReady();
    });
    w.init(std::move(router), 9123, 3, 8, false, LogLevel::INFO);
    w.run();
}
```
访问静态资源效果：
![image](https://user-images.githubusercontent.com/93439675/236688059-f46fc3bc-391f-4f2a-8daa-6cfc6469cb0e.png)

项目html资源全部来自：
[TinyWebServer](https://github.com/qinguoyi/TinyWebServer/tree/master)
