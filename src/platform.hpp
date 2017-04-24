#pragma once

#if __linux__
	#define ANTINET_linux
#elif defined(_WIN32) || defined(__CYGWIN__)
	#define ANTINET_windows
	#define UNICODE
	#define _UNICODE
	#if defined(__CYGWIN__)
		//http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/using.html
		#ifndef __USE_W32_SOCKETS
			#define __USE_W32_SOCKETS
		#endif
	#endif
#elif (__MACH__)
	#define ANTINET_macosx
#else
	#error Platform is not supported
#endif


// ===========================================================================================================
// TODO is this needed?
#if defined (__MINGW32__)
	#undef _assert
#endif


