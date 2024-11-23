#pragma once

#include <spdlog/spdlog.h>


class TriadLogs
{
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetTriadLogger() { return triadLogger; }

private:
    static std::shared_ptr<spdlog::logger> triadLogger;
};

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define __CALL_FROM__ std::format("[{}; {}:{}]", __FILENAME__, __FUNCTION__, __LINE__)

#ifdef NDEBUG

#define LOG_INFO(...)
#define LOG_DEBUG(...)
#define LOG_WARN(...)
#define LOG_ERROR(...)
#define LOG_CRIT(...)

#else

#define LOG_INFO(...)   ::TriadLogs::GetTriadLogger()->info(__VA_ARGS__)
#define LOG_DEBUG(...)  ::TriadLogs::GetTriadLogger()->debug(__VA_ARGS__)
#define LOG_WARN(...)   ::TriadLogs::GetTriadLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)  ::TriadLogs::GetTriadLogger()->error(__VA_ARGS__)
#define LOG_CRIT(...)   ::TriadLogs::GetTriadLogger()->critical(__VA_ARGS__)

#endif