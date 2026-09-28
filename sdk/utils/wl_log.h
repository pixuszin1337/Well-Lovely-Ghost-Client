#pragma once

#include <cstdio>
#include <cstdarg>
#include <windows.h>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace wl {

	inline void log(const char* fmt, ...)
	{
#ifndef NDEBUG
		static std::mutex mtx;
		std::lock_guard<std::mutex> lk(mtx);

		char buf[512];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		char appdata[MAX_PATH]{};
		if (GetEnvironmentVariableA("APPDATA", appdata, MAX_PATH) == 0)
			return;

		auto dir = std::filesystem::path(appdata) / "WellLovely";
		std::error_code ec;
		std::filesystem::create_directories(dir, ec);

		std::ofstream f(dir / "wl_debug.log", std::ios::app);
		if (!f)
			return;

		auto now = std::chrono::system_clock::now();
		std::tm tm{};
		auto t = std::chrono::system_clock::to_time_t(now);
		localtime_s(&tm, &t);
		f << std::put_time(&tm, "%H:%M:%S") << " " << buf << "\n";
#else
		(void)fmt;
#endif
	}

	inline void log_throttled(const char* tag, int ms, const char* fmt, ...)
	{
#ifndef NDEBUG
		static std::mutex mtx;
		std::lock_guard<std::mutex> lk(mtx);

		static std::string last_tag;
		static std::chrono::steady_clock::time_point last_time{};

		auto now = std::chrono::steady_clock::now();
		if (tag == last_tag && now - last_time < std::chrono::milliseconds(ms))
			return;

		last_tag = tag;
		last_time = now;

		char buf[512];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		log("%s", buf);
#else
		(void)tag; (void)ms; (void)fmt;
#endif
	}

}
