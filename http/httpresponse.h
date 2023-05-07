#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <string>
#include <unordered_map>

#include "httpinfo.h"

class HTTPResponse
{
public:
    HTTPResponse(HTTPBase::Version v, HTTPBase::StatusCode s, std::string desc)
        : keep_alive(true), is_access_resource(false), version(v), status(s),
          description(desc), headers(), file_path(), body(){};

    void setKeepAlive(bool is_keep_alive) { keep_alive = is_keep_alive; }
    void setIsAccessResource(std::string file_path_)
    {
        is_access_resource = true;
        file_path = file_path_;
    }

    void addHeader(std::string key, std::string value) { headers[key] = value; }
    void setBody(std::string body_) { body = body_; }
    bool isKeepAlive() { return keep_alive; }
    bool isAccessResource() { return is_access_resource; }

    const HTTPBase::Version& getVersion() { return version; }
    const std::string getVersionStr()
    {
        return std::to_string(version.major) + "." +
               std::to_string(version.minor);
    }
    const std::string getStatusStr()
    {
        return std::to_string(static_cast<int>(status)) + " " + description;
    }
    const HTTPBase::StatusCode& getStatus() { return status; }
    const std::string& getDescription() { return description; }
    const HTTPBase::Header& getHeaders() { return headers; }
    const std::string& getFilePath() { return file_path; }
    const std::string& getBody() { return body; }

    bool isHeadersSend() { return is_headers_send; }
    void setHeadersSend(bool is_send) { is_headers_send = is_send; }

private:
    bool keep_alive;
    bool is_access_resource;
    bool is_headers_send;

    HTTPBase::Version version;
    HTTPBase::StatusCode status;
    std::string description;
    HTTPBase::Header headers;
    std::string file_path;
    std::string body;
};
#endif   // __HTTPRESPONSE_H__