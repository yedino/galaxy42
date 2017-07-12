#pragma once

#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include <string>

#include "platform.hpp" // just to detect OS type, is it ANTINET_linux

#ifdef ANTINET_linux
	#include <linux/types.h>
	#include <cap-ng.h>
#endif

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

namespace capmodpp {

/*
 * Reports errors in capmodpp - errors when handling CAP Capability(8) privilages, program developers should probably
 * never catch this exception class, nor by catch(...) nor by catch on this class,
 * other then catching it in top of main() and the existing,
 * or catching it and doing an abort;
 * As this exception means program failed to execute expected security operations, and could be now in insecure state.
 * Unless when implementing optional security functions.
 * @warning If this is thrown then this usually means program is in insecure state and you should exit or abort (see above).
 * @style This does not inherit from std::exception, to make developers catch it explicitly if they want to.
 */
class capmodpp_error {
	public:
		capmodpp_error(const std::string & msg);
		virtual ~capmodpp_error()=default;
		virtual const char * what() const noexcept;
	private:
		std::string m_msg;
};

#ifdef ANTINET_linux

// ===========================================================================================================
/// @group low-level wrappers of native libcap-ng functions - adding more error handling, and exceptions
/// @{

/// We expect to never see longer Capability name (e.g. to detect strange values)
constexpr size_t max_expected_cap_name_length = 256;

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see int capng_have_capability(capng_type_t which, unsigned int capability);
/// @return 0 or 1 (this is guaranteed)
int secure_capng_have_capability(capng_type_t which, unsigned int capability);

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see capng_results_t capng_have_capabilities(capng_select_t set);
/// @return one of: {CAPNG_NONE, CAPNG_PARTIAL, CAPNG_FULL}, but will NOT return error value (this is guaranteed)
/// @return in addition, it is guaranteed that values representing more-then-none, are integers that are greater then 0 (x>0)
capng_results_t secure_capng_have_capabilities(capng_select_t set);

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see const char *capng_capability_to_name(unsigned int capability);
/// @return a not-nullptr valid c-string (this is guaranteed)
const char * secure_capng_capability_to_name(unsigned int capability);

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see int capng_name_to_capability(const char *name);
/// @param #name is the name capability; our function will assert that it is not badvalue like NULL
/// @return integer in range [0 .. get_cap_size] (this is guaranteed)
int secure_capng_name_to_capability(const char *name);

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @note this will: read current state from syscall (igoring cached libng state)
/// @see int capng_get_caps_process(void)
/// @return nothing is returned here (as it was used only to signal error)
void secure_capng_get_caps_process();

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see int capng_update(capng_act_t action, capng_type_t type,unsigned int capability);
/// @return nothing is returned here (as it was used only to signal error)
void secure_capng_update(capng_act_t action, capng_type_t type,unsigned int capability);

/// Wrapper around libcap-ng function, same API but it will throw capmodpp_error instead returning error-code.
/// @see int capng_apply(capng_select_t set);
void secure_capng_apply(capng_select_t set);


/// @}

// ===========================================================================================================

using cap_nr = unsigned int; ///< number of CAP (as defined by this OS/kernel), for libcap-ng, as in man capng_have_capability

cap_nr get_last_cap_nr() noexcept;
/// returns the number of last CAP in system (last still valid number)
/// this function can not fail, so it is safe to used in low-level validations

/// returns the number of CAPs in system, so this is +1 of last valid number, so loop to < this value
/// this function can not fail, so it is safe to used in low-level validations
cap_nr get_cap_size() noexcept;

///< returns string of CAP name, or "unknown" (unlikely, probably libcap-ng names it cap_n) or "invalid_cap_n" it out of range
std::string cap_nr_to_name(cap_nr nr);

// converts name of cap like NET_ADMIN into it's number
cap_nr cap_name_to_nr(const std::string & name);

enum class cap_area_type { eff,permit,inherit,bounding }; ///< the area of a CAP: effective, permited, inheritable and bounding

// ===========================================================================================================
// @group cap-permissions that program can HAVE (on/off)
// @{

enum class cap_perm { no=0, yes=1 }; ///< one capability-type state can be yes or no
bool is_true(cap_perm value) noexcept; ///< to convert to boolean, it will be TRUE if permision if granted (yes)

struct cap_statechange;

struct cap_state final { ///< one capability entire state, including effective, permited, inheritable and bounding
	cap_perm eff, permit, inherit, bounding; ///< this cap has enabled/disabled effective, permited, inheritable, effective
	cap_state();

	void apply_change(const cap_statechange & change); ///< apply given change to my state

	/**
 	 * is this CAP potentially usable as-is or by spawning a child, e.g. is it eff or permit or inherit,
	 * though NOT taking into accounts more advanced tricks (e.g. some other CAP allowing to rise any cap or create a new
	 * process with full CAPs etc
	 */
	bool is_usable() const;

	bool is_any() const; ///< TODO is anything enabled here

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
	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 0=oneliner, 10=only important CAP-values, 20=all
};

std::ostream & operator<<(std::ostream & ostr, const cap_state_map & obj);

cap_state_map read_process_caps();

// @}

// ===========================================================================================================
// @defgroup cap-permission CHANGES (enable / disable / leave unchanged)
// @{

/// one capability-type state CHANGE: can be to enable, disable, or leave unchanged
enum class cap_permchange { disable=0, enable=1, unchanged=2 };

std::ostream & operator<<(std::ostream & ostr, const cap_permchange & obj);

// @{ this are hard-typedef types, so that we can write function that demands args t_eff_value, t_permit_value,
// and it is static error in compilation to by mistake call it func(v_eff_enable, v_eff_enable) - the
// second arg is hard type error as t_permit_value is other type then t_eff_value
// with just a typedef/using, it would not be an error
struct t_eff_value final     { cap_permchange value; };
extern const t_eff_value v_eff_enable, v_eff_disable, v_eff_unchanged;

struct t_permit_value final  { cap_permchange value; };
extern const t_permit_value v_permit_enable, v_permit_disable, v_permit_unchanged;

struct t_inherit_value final { cap_permchange value; };
extern const t_inherit_value v_inherit_enable, v_inherit_disable, v_inherit_unchanged;

/// one capability entire state CHANGE, including effective, permited, inheritable and bounding
struct cap_statechange final {
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
struct cap_statechange_map final {
	std::map<cap_nr, cap_statechange> state;

	/// set the planned changed for cap named #capname to change value #change
	void set(const std::string & capname , cap_statechange change);

	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 10=only important CAP-values, 20=all
};

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_map & obj);

struct cap_statechange_full final {
	cap_statechange_map given; ///< changes for defined CAPs
	cap_statechange all_others; ///< how to change all others CAPs that are not mentioned in #given

	void security_apply_now() const; ///< SECURITY: apply now to current process the CAP changes as described by this object
	void print(std::ostream & ostr, int level=20) const; ///< Print to #ostr at verbosity #level 10=only important CAP-values, 20=all

	void set_given_cap(const std::string & capname , cap_statechange change);
	void set_all_others(cap_statechange change);
};

std::ostream & operator<<(std::ostream & ostr, const cap_statechange_full & obj);

// @}
// ===========================================================================================================

#else
	#pragma message "Library capmodpp is not supported on this OS (because linux Capabilities are not supported) - do not include this."
	// the file including this file, should put his #include into proper #ifdef
#endif
} // namespace
