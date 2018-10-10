#pragma once
// see also platform.hpp in top project
#include "../platform.hpp"

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

#if defined(__MACH__)
	#define stdplus_platform_macosx
#endif

#if defined(ANTINET_netbsd)
        #define stdplus_platform_netbsd
#endif

#if defined(ANTINET_openbsd)
        #define stdplus_platform_openbsd
#endif

#if defined(ANTINET_freebsd)
        #define stdplus_platform_freebsd
#endif

#if defined(EMPTY)
	#error Platform is not supported
#endif
