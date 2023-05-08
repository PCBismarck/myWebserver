#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <cstdint>
#include <string>
#include <unordered_map>

#include "httpinfo.h"

enum class ReqStatus {
    REQ_SUCCESS,
    REQ_ERROR,
    REQ_INCOMPLETE,
    REQ_PARSE_ERROR,
};

class HTTPRequest
{
public:
    // 使用正则表达式分析请求内容
    // 根据parse的结果来构造HTTPRequest对象
    ReqStatus parse(const char* buf, int len);

    HTTPBase::Method method() const { return m_method; }
    HTTPBase::Version version() const { return m_version; }
    const std::string& path() const { return m_path; }
    const std::string& host() const { return m_host; }
    uint32_t port() const { return m_port; }
    const HTTPBase::Header& headersAll() const { return m_headers; }
    std::string headers(std::string head_)
    {
        if (m_headers.find(head_) != m_headers.end()) {
            return m_headers[head_];
        }
        return "application/x-www-form-urlencoded";
    }
    std::string query(std::string key_)
    {
        if (m_query.find(key_) != m_query.end()) {
            return m_query[key_];
        }
        return "";
    }
    const std::string& body() const { return m_body; }
    bool isKeepAlive() const { return keep_alive; }

private:
    bool keep_alive;

    HTTPBase::Method m_method;
    HTTPBase::Version m_version;
    std::string m_path;
    std::string m_host;
    uint32_t m_port;
    HTTPBase::Header m_headers;
    std::string m_body;
    HTTPBase::Query m_query;
};

#endif   // __HTTPREQUEST_H__