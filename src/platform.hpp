#pragma once

// comment
// in c++17 ATTR_NODISCARD is in standard: [[nodiscard]]

#if __linux__
	#define ANTINET_linux
	#define ATTR_NODISCARD __attribute__((warn_unused_result))
#elif defined(_WIN32) || defined(__CYGWIN__)
	#define ANTINET_windows
	#define UNICODE
	#define _UNICODE
	#define ATTR_NODISCARD _Check_return_
	#if defined(__CYGWIN__)
		//http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/using.html
		#ifndef __USE_W32_SOCKETS
			#define __USE_W32_SOCKETS
		#endif
	#endif
#elif (__MACH__)
	#define ANTINET_macosx
	#define ATTR_NODISCARD __attribute__((warn_unused_result))
#else
	#error Platform is not supported
#endif
