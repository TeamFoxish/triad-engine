#include "Logs.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void TriadLogs::Init()
{
	logger = spdlog::basic_logger_mt("TriadEngine", "logs/LastLogs.txt", true);

	logger->sinks().push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());

	logger->set_level(spdlog::level::debug);

	spdlog::flush_every(std::chrono::seconds(3));
}

void TriadLogs::Term()
{
	logger->flush();
	logger.reset();
}
