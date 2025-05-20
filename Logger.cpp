#include <iostream>
#include <string>

#include "Logger.hpp"
#include "Timestamp.hpp"

Logger& Logger::instance()
{
    static Logger logger;
    return logger;
}
// 设置日志级别
int Logger::setLogLevel(int level)
{
    logLevel_ = level;
}
// 写日志 [logLevel] time : msg
void Logger::log(std::string& msg)
{
    switch (logLevel_)
    {
    case INFO:
        std::cout << "[INFO] ";        
        break;
    case ERROR:
        std::cout << "[ERROR] ";        
        break;
    case FATAL:
        std::cout << "[FATAL] ";        
        break;
    case DEBUG:
        std::cout << "[DEBUG] ";        
        break;
    default:
        break;
    }

    // 输出时间和msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}