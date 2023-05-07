#ifndef __CONFIGER_H__
#define __CONFIGER_H__

#include <string>
class Configer
{
public:
    Configer();
    ~Configer();

    int getPort() const;
    int getTimeout() const;
    int getThreadNum() const;

private:
    int m_port;
    int m_timeout;
    int m_threadNum;

    // 数据库相关
    std::string m_username;
    std::string m_password;
    std::string m_databaseName;
};

#endif   //__CONFIGER_H__