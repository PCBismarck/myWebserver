#ifndef __LOG_H__
#define __LOG_H__

#include <bits/types/time_t.h>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include "../threadpool/threadpool.h"

enum class LogLevel {
    INFO,
    DEBUG,
    WARN,
    ERROR,
    FATAL
};

class LogEvent
{
public:
    LogEvent() = default;
    // LogEvent(LogEvent&&) = default;

    LogEvent(LogLevel level_, time_t when_, std::string&& content_);

public:
    LogLevel level;
    time_t when;
    std::string content;
};

class Logger
{
public:
    static Logger* getInstance()
    {
        static Logger instance;
        return &instance;
    }
    ~Logger();

    void init(LogLevel level, int queue_size, std::string suffix = ".log",
              std::string path = "./LogFile");
    void write(LogLevel level_, time_t when_, std::string&& content_);

    bool empty() const;
    bool full() const;
    bool reopen();
    void close() { m_channels->close(); };
    bool isOpen() { return opened; };

private:
    void flush();
    void writeToFile(LogEvent& event);
    std::string addBaseInfo(LogLevel level, time_t when);

private:
    std::string m_suffix;
    std::string m_log_path;
    std::string m_log_name;

    bool opened = false;
    int max_queue_size;
    std::fstream file;
    std::mutex m_mtx;
    LogLevel m_level;
    std::unique_ptr<Channel<LogEvent>> m_channels;
    std::unique_ptr<std::thread> m_write_thread;
};

template <typename... Args>
std::string __format__(const char* format, Args... args)
{
    size_t length = std::snprintf(nullptr, 0, format, args...);
    if (length <= 0) {
        return "";
    }

    char* buf = new char[length + 1];
    std::snprintf(buf, length + 1, format, args...);

    std::string str(buf);
    delete[] buf;
    return std::move(str);
}

#define LOG_BASE(level, format, ...)                          \
    do {                                                      \
        Logger* logger = Logger::getInstance();               \
        if (logger->isOpen() && !logger->full()) {            \
            logger->write(level, time(nullptr),               \
                          __format__(format, ##__VA_ARGS__)); \
        }                                                     \
    } while (0);

#define LOG_INFO(format, ...)  LOG_BASE(LogLevel::INFO, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) LOG_BASE(LogLevel::DEBUG, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  LOG_BASE(LogLevel::WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG_BASE(LogLevel::ERROR, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) LOG_BASE(LogLevel::FATAL, format, ##__VA_ARGS__)

#endif   //__LOG_H__