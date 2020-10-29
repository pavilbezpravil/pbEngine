#pragma once

#include "pbe/Core/Base.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace pbe {

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
#define HZ_CORE_TRACE(...)	pbe::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define HZ_CORE_INFO(...)	pbe::Log::GetCoreLogger()->info(__VA_ARGS__)
#define HZ_CORE_WARN(...)	pbe::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define HZ_CORE_ERROR(...)	pbe::Log::GetCoreLogger()->error(__VA_ARGS__)
#define HZ_CORE_FATAL(...)	pbe::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client Logging Macros
#define HZ_TRACE(...)	pbe::Log::GetClientLogger()->trace(__VA_ARGS__)
#define HZ_INFO(...)	pbe::Log::GetClientLogger()->info(__VA_ARGS__)
#define HZ_WARN(...)	pbe::Log::GetClientLogger()->warn(__VA_ARGS__)
#define HZ_ERROR(...)	pbe::Log::GetClientLogger()->error(__VA_ARGS__)
#define HZ_FATAL(...)	pbe::Log::GetClientLogger()->critical(__VA_ARGS__)