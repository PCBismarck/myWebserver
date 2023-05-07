#include "log.h"
#include <bits/types/struct_tm.h>
#include <bits/types/time_t.h>
#include <cassert>
#include <cstddef>
#include <iomanip>
#include <ios>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <unistd.h>
#include <utility>

LogEvent::LogEvent(LogLevel level_, time_t when_, std::string&& content_)
    : level(level_), when(when_), content(content_)
{
    assert(level_ >= LogLevel::INFO && level_ <= LogLevel::FATAL);
}

Logger::~Logger()
{
    if (m_write_thread && m_write_thread->joinable()) {
        opened = false;
        m_channels->close();
        m_write_thread->join();
    }
    if (file.is_open()) {
        std::lock_guard<std::mutex> lock(m_mtx);
        // file << addBaseInfo(LogLevel::INFO, time(nullptr)) << "Logger
        // closed."
        //      << std::endl;
        file.close();
    }
}

void Logger::init(LogLevel level, int queue_size, std::string suffix,
                  std::string path)
{
    max_queue_size = queue_size;
    m_suffix = suffix;
    m_log_path = path;
    m_level = level;
    opened = true;
    assert(queue_size > 0);

    m_channels = std::make_unique<Channel<LogEvent>>();
    m_write_thread =
        std::move(std::make_unique<std::thread>(&Logger::flush, this));
    time_t t = time(nullptr);
    struct tm sys_time = *localtime(&t);

    if (m_log_path.back() != '/')
        m_log_path += "/";
    m_log_name = m_log_path + std::to_string(sys_time.tm_year + 1900) + "-" +
                 std::to_string(sys_time.tm_mon + 1) + "-" +
                 std::to_string(sys_time.tm_mday) + m_suffix;

    file.open(m_log_name, std::ios::out | std::ios::app);
    std ::cout << m_log_name << std::endl;
    assert(file.is_open());
}

// 将logevent写入队列
void Logger::write(LogLevel level_, time_t when_, std::string&& content_)
{
    LogEvent event(level_, when_, std::move(content_));
    if (m_channels->size() >= max_queue_size) {
        m_channels->notify_one();
    }
    m_channels->put(std::move(event));
}

bool Logger::empty() const { return m_channels->empty(); }

bool Logger::full() const { return m_channels->size() >= max_queue_size; }

bool Logger::reopen()
{
    if (!file.is_open())
        file.open(m_log_path + "/" + m_log_name, std::ios::app);
    return file.is_open();
}

// 持续向日志文件写入
void Logger::flush()
{
    while (true) {
        LogEvent event;
        if (!m_channels->take(event))
            break;
        writeToFile(event);
    }
}

void Logger::writeToFile(LogEvent& event)
{
    if (event.level < m_level)
        return;
    if (!file.is_open())
        reopen();
    assert(file.is_open());
    file << addBaseInfo(event.level, event.when) << event.content << std::endl;
}

std::string Logger::addBaseInfo(LogLevel level, time_t when)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) %
              1000;
    char time_str[20];
    std::strftime(time_str, 20, "%Y-%m-%d %H.%M.%S", std::localtime(&now_c));
    std::string buf;
    std::stringstream ss;
    ss << time_str << "." << ms.count() << " ";
    buf += ss.str();
    switch (level) {
    case LogLevel::INFO:
        buf += "[INFO] : ";
        break;
    case LogLevel::DEBUG:
        buf += "[DEBUG]: ";
        break;
    case LogLevel::WARN:
        buf += "[WARN] : ";
        break;
    case LogLevel::ERROR:
        buf += "[ERROR]: ";
        break;
    case LogLevel::FATAL:
        buf += "[FATAL]: ";
        break;
    default:
        buf += "[UNKNOW]:";
        break;
    }
    return buf;
}