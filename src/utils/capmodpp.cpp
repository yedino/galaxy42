
#include <platform.hpp>
#include "capmodpp.hpp"


#ifdef ANTINET_linux
	#include <linux/types.h>

	// uses libcap-ng
	#include <cap-ng.h>
#else
	#error "not supported"
#endif


namespace capmodpp {

std::ostream & operator<<(std::ostream & oss, const cap_perm & obj) {
	oss <<  ( (obj == cap_perm::yes) ? "YES" : "no" ) ;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_state & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_state_map & obj) {
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_permchange & obj) {
	switch (obj) {
		case cap_permchange::unchanged: oss<<"    same   "; break;
		case cap_permchange::enable:    oss<<"  +ENABLE+ "; break;
		case cap_permchange::disable:   oss<<" -disable- "; break;
	}
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange_map & obj) {
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

std::ostream & operator<<(std::ostream & oss, const cap_statechange_full & obj) {
	oss << obj.given << "\n" << "all_others: " << obj.all_others ;
	return oss;
}

}//namespace capmodpp
