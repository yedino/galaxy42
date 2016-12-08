

// need this to have the strerror_s - as in http://en.cppreference.com/w/c/string/byte/strerror point 3
// this must be on top, must be first inclusion of <string> (in this compilation / translation unit)
#define __STDC_WANT_LIB_EXT1__ 1
#include <string>
#if (!defined(__STDC_LIB_EXT1__))
	// TODO #warning not works on MSVC
	//#warning "It seems __STDC_LIB_EXT1__ is not set by implementation/compiler - and we need it to have strerror_s"
#endif

#include <syserror_use.hpp>

#include <cstring>
#include <sstream>


std::string errno_to_string(int errno_copy) {
	// [02/12/2016 15:43] for http://man7.org/linux/man-pages/man3/strerror.3.html  how exactly to test in C program with #if should I call XSI-compilant, or GNU, version of strerror_r
	// [02/12/2016 15:44] <izalove>            The XSI-compliant version is provided if:
	// [02/12/2016 15:44] <izalove>            (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE
	// [02/12/2016 15:44] <izalove>            Otherwise, the GNU-specific version is provided.

	const size_t buflen = 1024;
	char buf[buflen];
	std::memset(buf, 0, buflen); // extra guarantee no memleak

	#if ( \
		( (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE ) \
		||  ( defined(__MACH__) ) \
		||  ( defined(_WIN32) || defined(__CYGWIN__) ) \
	)
		// for linux in XSI mode
		// for mac os x
		// for windows - https://msdn.microsoft.com/en-us/library/51sah927.aspx

		#if ( defined(__linux__) || defined(__MACH__) )
			// Linus XSI and Mac:
			//  int strerror_r(int errnum, char *buf, size_t buflen);
			int result = strerror_r(errno_copy, buf, buflen);
		#elif ( defined(_WIN32) || defined(__CYGWIN__) )
			// Windows:
			// strerror_s( char *buffer, size_t numberOfElements, int errnum )
			int result = strerror_s( buf , buflen , errno_copy );
		#else
			#error "Unknown platform for strerror_r / strerror_s - please add IF for this platform here"
		#endif

		if (result != 0) { // on linux, mac, windows
			std::ostringstream oss;
			oss << "(errno_copy=" << errno_copy<< ", can not convert this to error message because result="<<result <<")";
			return oss.str();
		}

		buf[buflen-1]=0; // guarantee string terminates
		std::string ret(buf);
	#else

		// char *strerror_r(int errnum, char *buf, size_t buflen);  /* GNU-specific */
		char * result = strerror_r(errno_copy, buf, buflen);
		// result can point to our buf, or to some other (and immutable) string

		buf[buflen-1]=0; // guarantee string terminates (if result points to our buf, at least otherwise we just use their immutable C-string)
		std::string ret(result); // construct string from pointer (that is either our buf, or their immutable C-string)
	#endif

	return ret;
}

