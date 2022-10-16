#include "pch.h"
#include "log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace CBuild {

	std::shared_ptr<spdlog::logger> Log::logger;

	void Log::init() {

		spdlog::set_pattern("%^[%T] [%n]: %v%$");
		logger = spdlog::stdout_color_mt("CBuild");
		logger->set_level(spdlog::level::trace);

	}

}