
#pragma once

// see also platform.hpp in top project

#if __linux__
	#define stdplus_platform_linux

#elif defined(_WIN32) || defined(__CYGWIN__)

	#if defined (__MINGW32__)
		#define stdplus_platform_windows_mingw
	#else // MSVC
		#define stdplus_platform_windows_msvc
	#endif

	#define stdplus_platform_windows

	#if defined(__CYGWIN__)
		#define stdplus_platform_windows_cygwin
	#endif

#elif (__MACH__)

	#define stdplus_platform_macosx

#else

	#error Platform is not supported

#endif



