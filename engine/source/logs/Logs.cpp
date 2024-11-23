#include "Logs.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

std::shared_ptr<spdlog::logger> TriadLogs::triadLogger;

void TriadLogs::Init()
{
	triadLogger = spdlog::basic_logger_mt("TriadEngine", "logs/LastLogs.txt", true);

	triadLogger->sinks().push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());

	triadLogger->set_level(spdlog::level::debug);

	spdlog::flush_every(std::chrono::seconds(3));
}
