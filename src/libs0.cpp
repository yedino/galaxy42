// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "libs0.hpp"


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
