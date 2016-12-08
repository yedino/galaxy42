// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "libs0.hpp"
#include <boost/locale.hpp>

const char* expected_exception::what() const noexcept {
	return "expected_exception";
}

const char* expected_not_found::what() const noexcept {
	return "expected_not_found";
}


namespace stdplus {

std::string to_string(const std::string & v) {
	return v;
}

} // namespace

#if defined(_MSC_VER)
const char * gettext(const char * s) {
	return s;
}

char * bindtextdomain(const char * domainname, const char * dirname) {
	return  "not implemented";
}

char * textdomain(const char * domainname) {
	return "not in use";
}
#endif
