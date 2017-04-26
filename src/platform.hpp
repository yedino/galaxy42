#pragma once

// any errors about not-returning value from non-void function, will be hard errors,
// you can quiet them e.g. using DEAD_RETURN_I_AM_SURE();
#pragma GCC diagnostic error "-Wreturn-type"

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

	#define timegm _mkgmtime

	#include<sec_api/tchar_s.h>

	std::wstring carray_to_wstring(const char *cstr);
	// inspiration:
	// http://stackoverflow.com/questions/17258029/c-setenv-undefined-identifier-in-visual-studio/23616164#23616164
	int setenv(const char *name, const char *value, int overwrite) {
		int errcode = 0;

		std::wstring wname = carray_to_wstring(name);
		std::wstring wvalue = carray_to_wstring(value);

		if(!overwrite) {
			size_t envsize = 0;
			errcode = _wgetenv_s(&envsize, nullptr, 0, wname.c_str());
			if(errcode || envsize) return errcode;
		}
		return _wputenv_s(wname.c_str(), wvalue.c_str());
	}
	int unsetenv(const char *name) {
		return setenv(name, "", 1);
	}

#elif (__MACH__)
	#define ANTINET_macosx
	#define ATTR_NODISCARD __attribute__((warn_unused_result))
#else
	#error Platform is not supported
#endif


// ===========================================================================================================
// TODO is this needed?
#if defined (__MINGW32__)
	#undef _assert
#endif


