
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

	#if ( (_POSIX_C_SOURCE >= 200112L) && !  _GNU_SOURCE )  ||  (defined(__MACH__))
		// XSI
		//  int strerror_r(int errnum, char *buf, size_t buflen);
		int result = strerror_r(errno_copy, buf, buflen);
		// The XSI-compliant strerror_r() function returns 0 on success.  On
		// error, a (positive) error number is returned (since glibc 2.13), or
		// -1 is returned and errno is set to indicate the error (glibc versions
		// before 2.13). -- according to http://man7.org/linux/man-pages/man3/strerror.3.html

		if (result != 0) {
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

