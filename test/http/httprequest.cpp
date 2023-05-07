#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "../../http/httpinfo.h"
#include "../../http/httprequest.cpp"
#include "../../http/httprequest.h"

char reqbuf[] =
    "GET /hello HTTP/1.1\r\n"
    "Host: www.google.com:8080\r\n"
    "Content-Length: 11\r\n"
    "Server: GWS/2.0\r\n"
    "Date: Sat, 11 Jan 2003 02:44:04 GMT\r\n"
    "Content-Type: text/html\r\n"
    "Cache-control: private\r\n"
    "Set-Cookie: PREF=ID=73d4aef52e57bae9:TM=1042253044:LM=1042253044:S=SMCc_HRPCQiqy"
    "X9j; expires=Sun, 17-Jan-2038 19:14:07 GMT; path=/; domain=.google.com\r\n"
    "Connection: keep-alive\r\n\r\n"
    "hello world";

TEST_CASE("http reuqest parse test")
{
    HTTPRequest req;
    req.parse(reqbuf, sizeof(reqbuf));
    CHECK(req.method() == HTTPBase::Method::GET);
    CHECK(req.path() == "/hello");
    CHECK(req.version().major == 1);
    CHECK(req.version().minor == 1);
    CHECK(req.headers("Host") == "www.google.com:8080");
    CHECK(req.body() == "hello world");
}