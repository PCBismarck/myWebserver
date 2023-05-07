#include "httprequest.h"
#include <regex>
#include "httpinfo.h"

// 使用正则表达式分析请求内容
// 根据parse的结果来构造HTTPRequest对象
ReqStatus HTTPRequest::parse(const char* buf, int len)
{
    // std::regex_match
    std::regex request_line_regex("^([^ ]*) ([^ ]*) HTTP/(\\d*).(\\d*)\r\n");
    std::regex request_host_regex("^Host: ?(.*)\r\n");
    std::regex request_header_regex("^([^:]*): ?(.*)\r\n");
    std::cmatch result;
    const char* start = buf;
    // 解析请求行
    if (std::regex_search(start, result, request_line_regex)) {
        m_method = HTTPBase::str2method(
            std::string(result[1].first, result[1].second));
        m_path = std::string(result[2].first, result[2].second);
        m_version =
            HTTPBase::Version(atoi(result[3].first), atoi(result[4].first));
        start = result[0].second;
    } else {
        return ReqStatus::REQ_INCOMPLETE;
    }

    // 解析host port
    if (std::regex_search(start, result, request_host_regex)) {
        std::string total = std::string(result[1].first, result[1].second);
        m_headers["Host"] = total;
        if (total.find(':') != total.npos) {
            m_host = total.substr(0, total.find(':'));
            m_port = atoi(total.substr(total.find(':') + 1).c_str());
            if (m_port > 65535 || m_port < 1024) {
                return ReqStatus::REQ_ERROR;
            }
        } else {
            m_host = std::move(total);
            m_port = 80;
        }
        start = result[0].second;
    } else {
        return ReqStatus::REQ_INCOMPLETE;
    }

    // 解析header
    while (std::regex_search(start, result, request_header_regex)) {
        m_headers[std::string(result[1].first, result[1].second)] =
            std::string(result[2].first, result[2].second);
        start = result[0].second;
    }

    // 解析body
    if (std::regex_search(start, result, std::regex("(\r\n)+"))) {
        start = result[0].second;
        if (m_headers.find("Content-Length") != m_headers.end()) {
            int content_len = atoi(m_headers["Content-Length"].c_str());
            if (content_len > len - (start - buf)) {
                return ReqStatus::REQ_INCOMPLETE;
            }
            m_body = std::string(start, start + content_len);
        }
    }

    // 设置keep_alive
    if (m_headers.find("Connection") != m_headers.end() &&
        m_headers["Connection"] == "keep-alive") {
        keep_alive = true;
    } else {
        keep_alive = false;
    }
    return ReqStatus::REQ_SUCCESS;
}