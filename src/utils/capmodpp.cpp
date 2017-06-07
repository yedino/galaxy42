

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

typedef unsigned int cap_nr; ///< number of CAP, for libcap-ng, as in man capng_have_capability

enum class cap_perm { no=0, yes=1 }; ///< one capability-type state can be yes or no

std::ostream & operator<<(std::ostream & oss, const cap_perm & obj) {
	oss <<  ( (obj == cap_perm::yes) ? "YES" : "no" ) ;
	return oss;
}

struct cap_state { ///< one capability entire state, including effective, permited, inheritable and bounding
	cap_perm eff, permit, inherit, bounding; ///< this cap has enabled/disabled effective, permited, inheritable, effective
	cap_state() : eff(cap_perm::no), permit(cap_perm::no), inherit(cap_perm::no), bounding(cap_perm::no) { }
};

std::ostream & operator<<(std::ostream & oss, const cap_state & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

/**
 * map of states of permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=yes, permit=yes, inherit=no, bounding=no,
 * CAP_NET_RAW   eff=yes, permit=yes, inherit=no, bounding=no,
 */
struct cap_state_map {
	std::map<cap_nr, cap_state> state;
};

std::ostream & operator<<(std::ostream & oss, const cap_state_map & obj) {
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

/// one capability-type state CHANGE: can be to enable, disable, or leave unchanged
enum class cap_permchange { disable=0, enable=1, unchanged=2 };

std::ostream & operator<<(std::ostream & oss, const cap_permchange & obj) {
	switch (obj) {
		case cap_permchange::unchanged: oss<<"    same   "; break;
		case cap_permchange::enable:    oss<<"  +ENABLE+ "; break;
		case cap_permchange::disable:   oss<<" -disable- "; break;
	}
	return oss;
}

/// one capability entire state CHANGE, including effective, permited, inheritable and bounding
struct cap_statechange {
	/// this cap has separate CHANGE decissions, for each area: effective, permited, inheritable, effective
	cap_permchange eff, permit, inherit, bounding;
	cap_statechange() : eff(cap_permchange::unchanged), permit(cap_permchange::unchanged),
		inherit(cap_permchange::unchanged), bounding(cap_permchange::unchanged) { }
};
std::ostream & operator<<(std::ostream & oss, const cap_statechange & obj) {
	oss << "eff="<<obj.eff<<" permit="<<obj.permit<<" inherit="<<obj.inherit<<" bound="<<obj.bounding;
	return oss;
}

/**
 * map of states of CHANGES to permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=leave, permit=leave, inherit=disable, bounding=disable,
 * CAP_NET_RAW   eff=leave, permit=leave, inherit=disable, bounding=disable,
 */
struct cap_statechange_map {
	std::map<cap_nr, cap_state> state;
};

std::ostream & operator<<(std::ostream & oss, const cap_statechange_map & obj) {
	for(const auto & item : obj.state) {
		oss << item.first << ": " << item.second << "\n";
	}
	return oss;
}

struct cap_statechange_full {
	cap_statechange_map given; ///< changes for defined CAPs
	cap_permchange all_others; ///< how to change all others CAPs that are not mentioned in #given
};

std::ostream & operator<<(std::ostream & oss, const cap_statechange_full & obj) {
	oss << obj.given << "\n" << "all_others: " << obj.all_others ;
	return oss;
}



