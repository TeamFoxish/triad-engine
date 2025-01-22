#pragma once

#include <spdlog/spdlog.h>


class TriadLogs
{
public:
    static void Init();

    static void Term();

    static spdlog::logger& Logger() { return *logger; }

private:
    static inline std::shared_ptr<spdlog::logger> logger;
};

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __CALL_FROM__ std::format("[{}; {}:{}]", __FILENAME__, __FUNCTION__, __LINE__)

#if defined(NDEBUG) && !defined(EDITOR)

#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_CRIT(...)

#else

#define LOG_ENABLED

#define LOG_INFO(...)   ::TriadLogs::Logger().info(__VA_ARGS__)
#define LOG_DEBUG(...)  ::TriadLogs::Logger().debug(__VA_ARGS__)
#define LOG_WARN(...)   ::TriadLogs::Logger().warn(__VA_ARGS__)
#define LOG_ERROR(...)  ::TriadLogs::Logger().error(__VA_ARGS__)
#define LOG_CRIT(...)   ::TriadLogs::Logger().critical(__VA_ARGS__)

#endif