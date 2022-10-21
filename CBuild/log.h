#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace CBuild {

	class Log {
	private:
		static std::shared_ptr<spdlog::logger> logger;

	public:
		static void init();
		inline static std::shared_ptr<spdlog::logger>& get_logger() { return logger; }

	};

}

#define CBUILD_TRACE(...)::CBuild::Log::get_logger()->trace(__VA_ARGS__)
#define CBUILD_INFO(...)::CBuild::Log::get_logger()->info(__VA_ARGS__)
#define CBUILD_WARN(...)::CBuild::Log::get_logger()->warn(__VA_ARGS__)
#define CBUILD_ERROR(...)::CBuild::Log::get_logger()->error(__VA_ARGS__)
#define CBUILD_CRITICAL(...)::CBuild::Log::get_logger()->critical(__VA_ARGS__)