#pragma once

#include <iostream>
#include <map>
#include <sstream>

namespace capmodpp {

/**
 * @file capmodpp - Capability Modifications CPP library
 * @owner rfree
 *
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
enum class cap_area_type { eff,permit,inherit,bounding }; ///< the area of a CAP: effective, permited, inheritable and bounding

struct cap_state { ///< one capability entire state, including effective, permited, inheritable and bounding
	cap_perm eff, permit, inherit, bounding; ///< this cap has enabled/disabled effective, permited, inheritable, effective
	cap_state();
};

std::ostream & operator<<(std::ostream & oss, const cap_state & obj);

std::ostream & operator<<(std::ostream & oss, const cap_perm & obj);

/**
 * map of states of permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=yes, permit=yes, inherit=no, bounding=no,
 * CAP_NET_RAW   eff=yes, permit=yes, inherit=no, bounding=no,
 */
struct cap_state_map {
	std::map<cap_nr, cap_state> state;
};

std::ostream & operator<<(std::ostream & oss, const cap_state_map & obj);

cap_state_map read_process_caps();

/// one capability-type state CHANGE: can be to enable, disable, or leave unchanged
enum class cap_permchange { disable=0, enable=1, unchanged=2 };

std::ostream & operator<<(std::ostream & oss, const cap_permchange & obj);

/// one capability entire state CHANGE, including effective, permited, inheritable and bounding
struct cap_statechange {
	/// this cap has separate CHANGE decissions, for each area: effective, permited, inheritable, effective
	cap_permchange eff, permit, inherit, bounding;

	cap_statechange();
	/// applies to given area (e.g. permited) given value (e.g. enable).
	cap_statechange & set(cap_area_type area, cap_permchange value);
};

std::ostream & operator<<(std::ostream & oss, const cap_statechange & obj);

/**
 * map of states of CHANGES to permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=leave, permit=leave, inherit=disable, bounding=disable,
 * CAP_NET_RAW   eff=leave, permit=leave, inherit=disable, bounding=disable,
 */
struct cap_statechange_map {
	std::map<cap_nr, cap_state> state;
};

std::ostream & operator<<(std::ostream & oss, const cap_statechange_map & obj);

struct cap_statechange_full {
	cap_statechange_map given; ///< changes for defined CAPs
	cap_permchange all_others; ///< how to change all others CAPs that are not mentioned in #given
};

std::ostream & operator<<(std::ostream & oss, const cap_statechange_full & obj);

}


