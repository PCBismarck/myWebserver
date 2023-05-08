#include <iostream>
#include <regex>
#include <stdlib.h>
#include <string>
#include <unordered_map>

using namespace std;

void testQuery()
{
    std::string m_path("/hello?name=world&age=18&blank=");
    std::unordered_map<std::string, std::string> m_query;
    std::cmatch result;
    if (m_path.find('?') != m_path.npos) {
        std::string query = m_path.substr(m_path.find('?'));
        m_path = m_path.substr(0, m_path.find('?'));
        const char* query_start = query.c_str();
        std::regex request_query_regex("[?&]([^&= ]*)=([^&= ]*)");
        while (std::regex_search(query_start, result, request_query_regex)) {
            std::string key = std::string(result[1].first, result[1].second);
            std::string value = std::string(result[2].first, result[2].second);
            m_query[key] = value;
            query_start = result[0].second;
        }
    }
}

void testReplace()
{
    unordered_map<string, string> map;
    map["/static"] = "./resource";
    map["/hello"] = "./world";
    std::string s("/static/index.html");
    string s2("/hello/index.html");
    for (auto& [pre, to] : map) {
        std::regex p("^" + pre);
        if (std::regex_search(s, p)) {
            std::cout << s << "match" << std::endl;
            auto text = std::regex_replace(s, p, to);
            cout << "new path: " << text << endl;
        }
    }
    for (auto& [pre, to] : map) {
        std::regex p("^" + pre);
        if (std::regex_search(s2, p)) {
            std::cout << s2 << "match" << std::endl;
            auto text = std::regex_replace(s2, p, to);
            cout << "new path: " << text << endl;
        }
    }
}

void testHttp()
{
    std::unordered_map<std::string, std::string> header;
    std::string txt("GET /hello HTTP/1.1");
    // HTTP/1.1 200 OK
    char resp[] = "\
GET /hello HTTP/1.1\r\n\
Host: www.google.com:8080\r\n\
Content-Length: 3059\r\n\
Server: GWS/2.0\r\n\
Date: Sat, 11 Jan 2003 02:44:04 GMT\r\n\
Content-Type: text/html\r\n\
Cache-control: private\r\n\
Set-Cookie: PREF=ID=73d4aef52e57bae9:TM=1042253044:LM=1042253044:S=SMCc_HRPCQiqy\
X9j; expires=Sun, 17-Jan-2038 19:14:07 GMT; path=/; domain=.google.com\r\n\
Connection: keep-alive\r\n\r\n\
hello world\r\n\r\n";

    char* buf = (char*)txt.c_str();
    std::regex request_line_regex("^([^ ]*) ([^ ]*) HTTP/(\\d*).(\\d*)\r\n");
    std::regex pattern("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::regex headers("([^ ]*): (.*)\r\n");
    std::regex request_header_regex("^([^:]*): ?(.*)\r\n");
    std::regex respline("(HTTP/[^ ]* \\d* [^ ]*)\r\n");

    std::regex request_host_regex("^Host: ?(.*)\r\n");
    std::smatch result;
    std::cmatch cresult;

    const char* start = resp;
    const char* temp;
    if (regex_search(start, cresult, request_line_regex)) {
        cout << string(cresult[0].first, cresult[0].second) << " ";
        int major = atoi(cresult[3].first), minor = atoi(cresult[4].first);
        start = cresult[0].second;   // 更新搜索起始位置,搜索剩下的字符串
    }
    if (regex_search(start, cresult, request_host_regex)) {
        cout << string(cresult[0].first, cresult[0].second) << endl;
        string total = string(cresult[1].first, cresult[1].second);
        string host;
        int port;
        if (total.find(':') != total.npos) {
            host = total.substr(0, total.find(':'));
            port = atoi(total.substr(total.find(':') + 1).c_str());
        }
        start = cresult[0].second;
    }
    cout << endl << endl;
    unordered_map<string, string> head;
    while (regex_search(start, cresult, request_header_regex)) {
        head[std::string(cresult[1].first, cresult[1].second)] =
            string(cresult[2].first, cresult[2].second);
        start = cresult[0].second;   // 更新搜索起始位置,搜索剩下的字符串
    }
    for (auto& [k, v] : head) {
        cout << k << " : " << v << endl;
    }
    string body(start);
    cout << endl << head["Content-Length"] << endl;
}

int main(void)
{
    // testReplace();
    testQuery();
    return 0;
}