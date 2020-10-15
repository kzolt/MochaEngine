#pragma once

#include "Base.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Mocha {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core Logging Macros
#define MC_CORE_TRACE(...)			Mocha::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define MC_CORE_INFO(...)			Mocha::Log::GetCoreLogger()->info(__VA_ARGS__)
#define MC_CORE_WARN(...)			Mocha::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define MC_CORE_ERROR(...)			Mocha::Log::GetCoreLogger()->error(__VA_ARGS__)
#define MC_CORE_FATAL(...)			Mocha::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client Logging Macros
#define MC_TRACE(...)				Mocha::Log::GetClientLogger()->trace(__VA_ARGS__)
#define MC_INFO(...)				Mocha::Log::GetClientLogger()->info(__VA_ARGS__)
#define MC_WARN(...)				Mocha::Log::GetClientLogger()->warn(__VA_ARGS__)
#define MC_ERROR(...)				Mocha::Log::GetClientLogger()->error(__VA_ARGS__)
#define MC_FATAL(...)				Mocha::Log::GetClientLogger()->critical(__VA_ARGS__)