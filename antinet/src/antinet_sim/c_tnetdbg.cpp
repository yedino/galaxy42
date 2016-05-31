#include "c_tnetdbg.hpp"

#include <string>
#include <cstring>

const char * debug_shorten__FILE__(const char * name) {
	const char *p1 = name;
	const char *p2 = p1;

	static const std::string target("antinet");

	while (true) {
		p2 = std::strchr(p1, '/');
		if (p2==nullptr) return name; // not matched
		if (*p2=='\0') return name; // not matched

		// p2 is valid

		if (std::string(p1,p2) == target) return p2;

		p1 = p2+1;
		if (*p1=='\0') return name; // not matched
	}

}


std::string debug_this() {
	return "(global) ";
}

