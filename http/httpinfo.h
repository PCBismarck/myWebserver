#ifndef __HTTPINFO_H__
#define __HTTPINFO_H__

#include <string>
#include <unordered_map>

namespace HTTPBase
{
using Header = std::unordered_map<std::string, std::string>;
using Query = std::unordered_map<std::string, std::string>;

struct Version {
    int major;
    int minor;
    bool operator<(const Version& rhs) const
    {
        return major < rhs.major || (major == rhs.major && minor < rhs.minor);
    }
    Version() : major(1), minor(1) {}
    Version(int major_, int minor_) : major(major_), minor(minor_) {}
};

enum class Method {
    UNKNOWN,
    GET,
    POST,
    PUT,
    DELETE
};

inline Method str2method(const std::string& method_str)
{
    if (method_str == "GET")
        return Method::GET;
    if (method_str == "POST")
        return Method::POST;
    if (method_str == "PUT")
        return Method::PUT;
    if (method_str == "DELETE")
        return Method::DELETE;
    return Method::UNKNOWN;
};

inline std::string method2str(Method method)
{
    switch (method) {
    case Method::GET:
        return "GET";
    case Method::POST:
        return "POST";
    case Method::PUT:
        return "PUT";
    case Method::DELETE:
        return "DELETE";
    default:
        return "UNKNOWN";
    }
};

enum class StatusCode {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_ERROR = 500,
};
}   // namespace HTTPBase

#endif   // __HTTPINFO_H__