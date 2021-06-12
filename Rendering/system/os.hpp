#pragma once

namespace system
{
#if defined(_WIN32) || defined(_WIN64)
	inline constexpr bool OS_WINDOWS = true;
#define SYSTEM_OS_WINDOWS
#else
	inline constexpr bool OS_WINDOWS = false;
#endif

#if defined(__APPLE__) || defined(__MACH__)
	inline constexpr bool OS_APPLE = true;
#define SYSTEM_OS_APPLE
#else
	inline constexpr bool OS_APPLE = false;
#endif

#if defined(__linux__)
	inline constexpr bool OS_LINUX = true;
#define SYSTEM_OS_LINUX
#else
	inline constexpr bool OS_LINUX = false;
#endif

#if defined(__FreeBSD__)
	inline constexpr bool OS_FREEBSD = true;
#define SYSTEM_OS_FREEBSD
#else
	inline constexpr bool OS_FREEBSD = false;
#endif

#if defined(__ANDROID__)
	inline constexpr bool OS_ANDROID = true;
#define SYSTEM_OS_ANDROID
#else
	inline constexpr bool OS_ANDROID = false;
#endif
}
