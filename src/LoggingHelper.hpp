#ifndef LOGGINGHELPER_H
#define LOGGINGHELPER_H

#include <rtt/TaskContext.hpp>

class LoggingHelper
{
    const int DEFAULT_LOG_BUFFER_SIZE;
public:
    LoggingHelper();
    bool logAllPorts(RTT::TaskContext *context, const std::vector<std::string> excludeList = std::vector<std::string>());
    bool logAllTask();
};

#endif // LOGGINGHELPER_H
