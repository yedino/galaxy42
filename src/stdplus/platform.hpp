
#pragma once

// see also platform.hpp in top project

#if defined(__linux__)
	#define stdplus_platform_linux
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
	#if defined (__MINGW32__)
		#define stdplus_platform_windows_mingw
	#else // MSVC
		#define stdplus_platform_windows_msvc
	#endif

	#define stdplus_platform_windows

	#if defined(__CYGWIN__)
		#define stdplus_platform_windows_cygwin
	#endif
#endif

#if (__MACH__)
	#define stdplus_platform_macosx
#endif

#if defined (__OpenBSD__)
        #define stdplus_platform_openbsd
#endif

#if defined (EMPTY)
	#error Platform is not supported
#endif
