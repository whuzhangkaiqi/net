#pragma once

#include <string>

#include "noncopyable.hpp"

// LOG_INFO("%s, %d", "hello", 123);
#define LOG_INFO(logformatmsg, ...)                       \
    do                                                    \
    {                                                     \
        Logger logger = Logger::instance();               \
        logger.setLogLevel(INFO);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logformatmsg, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0);
#define LOG_ERROR(logformatmsg, ...)                       \
    do                                                    \
    {                                                     \
        Logger logger = Logger::instance();               \
        logger.setLogLevel(ERROR);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logformatmsg, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0);
#define LOG_FATAL(logformatmsg, ...)                       \
    do                                                    \
    {                                                     \
        Logger logger = Logger::instance();               \
        logger.setLogLevel(FATAL);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logformatmsg, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0);
#ifdef MUDEBUG
#define LOG_DEBUG(logformatmsg, ...)                       \
    do                                                    \
    {                                                     \
        Logger logger = Logger::instance();               \
        logger.setLogLevel(DEBUG);                         \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, logformatmsg, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0);
#else #define LOG_DEBUG(logformatmsg, ...)
#endif

// 定义日志的级别 INFO WARN ERROR FATAL DEBUG
enum LogLevel
{
    INFO, // 普通信息
    ERROR, // 错误信息
    FATAL, // core 信息
    DEBUG, //调试信息
};

// 定义日志类，只需要按照单例模式实现即可
class Logger : noncopyable
{
    public:
        // 用来获取唯一的日志实例对象
        static Logger& instance();
        // 设置日志级别
        int setLogLevel(int level);
        // 写日志
        void log(std::string& msg);

    protected:
    private:
        // 日志级别
        int logLevel_;
        // 构造函数私有化
        Logger();
};
