// #include <../../thre>
#include <string>
#include <unistd.h>
#include <utility>
#include "../../log/log.cpp"
#include "../../threadpool/threadpool.cpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("Log Test")
{
    {
        Logger* logger = Logger::getInstance();
        logger->init(LogLevel::INFO, 1000, ".log", "./test/log_test/log");
        // LogEvent event(LogLevel::INFO, time(nullptr), "test");
        logger->write(LogLevel::INFO, time(nullptr), "test");
        LOG_FATAL("%s", "fatal test");
        LOG_INFO("%s", "info test");
        LOG_ERROR("%s", "content test");
        LOG_WARN("%s", "warn test");
        LOG_DEBUG("string construct test [%d]", 1);
        std::string buf = "string test";
        LOG_DEBUG("%s", buf.c_str());
        CHECK(logger->empty() == false);
        CHECK(logger->full() == false);
        CHECK(logger->reopen() == true);
        logger->write(LogLevel::INFO, time(nullptr), "test2");
    }
    LOG_INFO("%s", "info test 2");
}
