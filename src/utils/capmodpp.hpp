#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <vector>

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

// ===========================================================================================================

typedef unsigned int cap_nr; ///< number of CAP (as defined by this OS/kernel), for libcap-ng, as in man capng_have_capability

cap_nr get_last_cap_nr(); ///< returns the number of last CAP in system (last still valid number)
cap_nr get_cap_size(); ///< returns the number of CAPs in system, so this is +1 of last valid number, so loop to < this value

///< returns string of CAP name, or "unknown" (unlikely, probably libcap-ng names it cap_n) or "invalid_cap_n" it out of range
std::string cap_nr_to_name(cap_nr nr);

// converts name of cap like NET_ADMIN into it's number
cap_nr cap_name_to_nr(const std::string & name);

enum class cap_area_type { eff,permit,inherit,bounding }; ///< the area of a CAP: effective, permited, inheritable and bounding

// ===========================================================================================================
// @group cap-permissions that program can HAVE (on/off)
// @{

enum class cap_perm { no=0, yes=1 }; ///< one capability-type state can be yes or no

struct cap_statechange;

struct cap_state { ///< one capability entire state, including effective, permited, inheritable and bounding
	cap_perm eff, permit, inherit, bounding; ///< this cap has enabled/disabled effective, permited, inheritable, effective
	cap_state();

	void apply_change(const cap_statechange & change); ///< apply given change to my state

	/// is this CAP potentially usable as-is or by spawning a child, e.g. is it eff or permit or inherit,
	/// though NOT taking into accounts more advanced tricks (e.g. some other CAP allowing to rise any cap or create a new
	/// process with full CAPs etc
	bool is_usable() const;
};

std::ostream & operator<<(std::ostream & ostr, const cap_state & obj);

std::ostream & operator<<(std::ostream & ostr, const cap_perm & obj);

/**
 * map of states of permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=yes, permit=yes, inherit=no, bounding=no,
 * CAP_NET_RAW   eff=yes, permit=yes, inherit=no, bounding=no,
 */
struct cap_state_map {
	std::map<cap_nr, cap_state> state;
	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 10=only important CAP-values, 20=all
};

std::ostream & operator<<(std::ostream & ostr, const cap_state_map & obj);

cap_state_map read_process_caps();

// @}

// ===========================================================================================================
// @group cap-permission CHANGES (enable / disable / leave unchanged)
// @{

/// one capability-type state CHANGE: can be to enable, disable, or leave unchanged
enum class cap_permchange { disable=0, enable=1, unchanged=2 };

std::ostream & operator<<(std::ostream & ostr, const cap_permchange & obj);

struct t_eff_value     { cap_permchange value; };
extern const t_eff_value v_eff_enable, v_eff_disable, v_eff_unchanged;

struct t_permit_value  { cap_permchange value; };
extern const t_permit_value v_permit_enable, v_permit_disable, v_permit_unchanged;

struct t_inherit_value { cap_permchange value; };
extern const t_inherit_value v_inherit_enable, v_inherit_disable, v_inherit_unchanged;

/// one capability entire state CHANGE, including effective, permited, inheritable and bounding
struct cap_statechange {
	/// this cap has separate CHANGE decissions, for each area: effective, permited, inheritable, effective
	cap_permchange eff, permit, inherit, bounding;

	cap_statechange();
	cap_statechange(t_eff_value given_eff, t_permit_value given_permit, t_inherit_value given_inherit);
	/// applies to given area (e.g. permited) given value (e.g. enable).
	cap_statechange & set(cap_area_type area, cap_permchange value);

	bool is_change() const; ///< is this doing any change to any area?
};

std::ostream & operator<<(std::ostream & ostr, const cap_statechange & obj);

/**
 * map of states of CHANGES to permissions
 * logically it's field #state works like a table like:
 * CAP_NET_ADMIN eff=leave, permit=leave, inherit=disable, bounding=disable,
 * CAP_NET_RAW   eff=leave, permit=leave, inherit=disable, bounding=disable,
 */
struct cap_statechange_map {
	std::map<cap_nr, cap_statechange> state;

	/// set the planned changed for cap named #capname to change value #change
	void set(const std::string & capname , cap_statechange change);

	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 10=only important CAP-values, 20=all
};

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_map & obj);

struct cap_statechange_full {
	cap_statechange_map given; ///< changes for defined CAPs
	cap_statechange all_others; ///< how to change all others CAPs that are not mentioned in #given

	void security_apply_now(); ///< SECURITY: apply now to current process the CAP changes as described by this object
	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 10=only important CAP-values, 20=all

	void set_given_cap(const std::string & capname , cap_statechange change);
	void set_all_others(cap_statechange change);
};

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_full & obj);

// @}
// ===========================================================================================================

}


