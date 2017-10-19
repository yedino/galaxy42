#pragma once

#include <climits> // for CHAR_BITS

// test for little/big edian is in src/c_ndp.cpp

// any errors about not-returning value from non-void function, will be hard errors,
// you can quiet them e.g. using DEAD_RETURN();
#pragma GCC diagnostic error "-Wreturn-type"

static_assert( CHAR_BIT == 8 , "This code requires CHAR_BIT == 8.");
static_assert( UCHAR_MAX == 255 , "This code requires UCHAR_MAX == 255.");

// see also stdplus/platform.hpp

// in c++17 ATTR_NODISCARD is in standard: [[nodiscard]]
#if __linux__
	#define ANTINET_linux
	#define ATTR_NODISCARD __attribute__((warn_unused_result))
#elif defined(_WIN32) || defined(__CYGWIN__)
	#if defined (__MINGW32__)
		#define ATTR_NODISCARD __attribute__((warn_unused_result))
	#else // MSVC
		#define ATTR_NODISCARD _Check_return_
	#endif
	#define ANTINET_windows
	#if defined(__CYGWIN__)
		//http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/using.html
		#ifndef __USE_W32_SOCKETS
			#define __USE_W32_SOCKETS
		#endif
	#endif
	#include <string>
	std::wstring cstring_to_wstring(const char *cstr);
	// inspiration:
	// http://stackoverflow.com/questions/17258029/c-setenv-undefined-identifier-in-visual-studio/23616164#23616164
	int setenv(const char *name, const char *value, int overwrite);

	int unsetenv(const char *name);

#elif (__MACH__)
	#define ANTINET_macosx
	#define ATTR_NODISCARD __attribute__((warn_unused_result))
#else
	#error Platform is not supported
#endif


// ===========================================================================================================

#if ( ! defined( ANTINET_windows ))
	// works on mac, linux
	#define ANTINET_socket_sockopt // we should set sockopt e.g. timeout options
#else
	#define ANTINET_cancelio // we should use window's cancelIO/family
#endif

// ===========================================================================================================

