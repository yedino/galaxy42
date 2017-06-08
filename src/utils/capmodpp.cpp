
#include <platform.hpp>
#include "capmodpp.hpp"


#ifdef ANTINET_linux
	#include <linux/types.h>

	// uses libcap-ng
	#include <cap-ng.h>
#else
	#error "not supported"
#endif


/**
 * Motivation, we want to apply following change to capabilities:
 *
 * CAP_NET_ADMIN eff=leave, permit=leave, inherit=disable, bounding=disable,
 * CAP_NET_RAW   eff=leave, permit=leave, inherit=disable, bounding=disable,
 * all_others:   eff=disable, permit=disable, inherit=disable, bounding=disable,
 *
 * So in short: we want to leave CAP NET_ADMIN as effective and permitet (but disable it's inheriting and bounding set),
 * same for CAP NET_RAW, and to all other CAPs we want to fully disable them.
 *
 * So we need to
 * 1) express such set of changes - type cap_statechange_full
 */
namespace capmodpp
{

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
