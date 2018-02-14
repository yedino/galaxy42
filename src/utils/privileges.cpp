
/**
 * @file privileges controll, using also libcap-ng
 * @note libcap-ng is https://people.redhat.com/sgrubb/libcap-ng/index.html [backup http://archive.is/CQWQE],
 * there is also https://github.com/Distrotech/libcap-ng/
 */

#include "privileges.hpp"
#include "tnetdbg.hpp"
#include "platform.hpp"
#include <thread>
#include <map>
#include <iostream>
#include "datastore.hpp"


#ifdef ANTINET_linux
	#include <linux/types.h>
	#include <pwd.h>
	#include <unistd.h>

	// uses libcap-ng
	#include <cap-ng.h>

	#include "utils/capmodpp.hpp"
#endif


namespace my_cap {
#ifdef ANTINET_linux
/**
 * Returns summary of allowed CAPs, also UID/GID, and possibly other details
 * In case of errors with checking state it should usually return proper information in the string, not throw.
 * @param verbose if yes then produce one-liner short info, else more detailed text (with \n endlines)
 */
std::string get_security_info(bool verbose) noexcept {
	try {
		std::ostringstream oss;
		try {
			oss << "UID="<<getuid()<<"(EUID="<<geteuid()<<") GID="<<getgid()<<"(EGID="<<getegid()<<")" ;
		} catch(const std::runtime_error ex) {
			oss << "[error reading GID/UID: " << ex.what() << "]";
		}
		oss<<" ";
		try {
			auto caps_now = capmodpp::read_process_caps();
			caps_now.print(oss, 0); // print one-liner
			if (verbose) {
				oss << "\nCap details:\n";
				caps_now.print(oss, 20);
			}
		} catch(const std::runtime_error ex) {
			oss << "[error reading caps: " << ex.what() << "]";
		}
		return oss.str();
	} catch(...) {
		_erro("Unexpected error");
		_check_abort(0); // trying to rethrow, will cause abort probably (if noexcept)
	}
}


void print_security_info() {
	_mark("Security info: " << get_security_info() );
}

/**
 * Applies a CAPs change given in #change, can do debug before/after.
 */
static void security_apply_cap_change(const capmodpp::cap_statechange_full & change) {
	_note("Caps (before): " << get_security_info(true) );
	_clue("Caps (will apply): " << change);
	change.security_apply_now();
	_note("Caps (after):  " << get_security_info(true) );
}
#endif

static bool do_we_need_to_change_uid_or_gid() {
	#ifdef ANTINET_linux
	uid_t uid = getuid();
	uid_t euid = geteuid();
	_note("uid="<<uid<<" euid="<<geteuid());
	if (uid != euid) {
		// see @warnings in .hpp
		_erro("UID is different then EUID, is this program set chmod SUID? This is not supported currently. Aborting.");
		std::abort(); // ! something is meesed up with security
	}
	if (euid==0) return true;
	if (uid==0) return true;
	return false;
	#else
		_note("This security operation is not available on this system, ignoring");
		return false;
	#endif
}

/**
 * If we are root (UID) then drops from root back to regular user who gained root by sudo, retains current CAPs
 * @warning NOT guaranteed to support double sudo (if user did e.g. sudo ./script and script does sudo ./program)
 * @pre Process must have CAPs needed for this special UID change: CAP_SETUID, CAP_SETGID, CAP_SETPCAP, CAP_CHOWN
 * @post User is not-root (not UID 0) or else exception is thrown
 * @return struct with variables that might be changed after droop root (for expmple string with regular user home path
 * if root is successfully dropped else empty string).
 */
static t_changes_from_sudo security_drop_root_from_sudo() {
	_fact("Dropping root (if we are root)");
	t_changes_from_sudo changes;

	#ifdef ANTINET_linux
	if ( ! do_we_need_to_change_uid_or_gid()) {
		_note("We are not root anyway");
		changes.m_home_dir = ""; // for now, set the default behaviour of datastore
		return changes;
	} // not root

	_clue("Yes, will change UID/GID");
	_note("Caps (when changing user): " << get_security_info() );

	const char* sudo_user_env = std::getenv("SUDO_USER");
	if (sudo_user_env == nullptr) throw std::runtime_error("SUDO_USER env is not set") ; // get env error
	std::string sudo_user_name(sudo_user_env);
	_info("sudo user is " << sudo_user_env);
	struct passwd *pw = getpwnam(sudo_user_name.c_str());
	if (pw == nullptr) {
		_fact("getpwnam error");
		throw std::system_error(std::error_code(errno, std::system_category()));
	}
	uid_t normal_user_uid = pw->pw_uid;
	gid_t normal_user_gid = pw->pw_gid;
	if (normal_user_uid == 0) throw std::runtime_error("The sudo was called by root");
	_fact("try to change uid to " << normal_user_uid);
	_fact("try to change gid to " << normal_user_gid);

	capmodpp::secure_capng_get_caps_process(); // causes libng to initialize, if not already
	int ret = capng_change_id(normal_user_uid, normal_user_gid, static_cast<capng_flags_t>(CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING));
	// TODO bounding set is NOT cleared!

	if (ret != 0) {
		_erro("capng_change_id error: " << ret);
		throw std::system_error(std::error_code(), "capng_change_id error, return value: " + std::to_string(ret));
	}
	_fact("UID/GID change done");
	_note("Caps (after changing user): " << get_security_info() );

	if (pw->pw_dir != nullptr)
		changes.m_home_dir = pw->pw_dir;
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif

	if (do_we_need_to_change_uid_or_gid()) {
		_erro("Something is wrong, we tried to remove root UID/GID but stil it is not done. Aborting.");
		std::abort();
	}
	return changes;
}

void drop_privileges_on_startup() {
	#ifdef ANTINET_linux
	_fact("Dropping privileges - on startup");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_RAW", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used
	// change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used

	if (do_we_need_to_change_uid_or_gid()) {
		// to drop root. is this really needed like that? needs more review.
		_fact("Leaving SETUID/SETGID (and PCAP) caps, to allow droping root UID later");
		change.set_given_cap("SETUID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("SETGID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("SETPCAP", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("CHOWN", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	}

	security_apply_cap_change(change);
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}


void drop_privileges_after_tuntap() {
	#ifdef ANTINET_linux
	_fact("Dropping privileges - after tuntap");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_given_cap("NET_RAW", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	// change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});

	security_apply_cap_change(change);

	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}

void drop_privileges_before_mainloop() {
	#ifdef ANTINET_linux
	_fact("Dropping privileges - before mainloop");
	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	security_apply_cap_change(change);
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}

void verify_privileges_are_as_for_mainloop() {
	try {
	#ifdef ANTINET_linux
	_goal("Verifying privileges are dropped - before entering the main loop");

	try {
		bool have_any_cap = (capmodpp::secure_capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE);
		if (have_any_cap) {
			_erro("We still have some CAP capability - when we expected to have none by now. Aborting.");
			std::abort(); // <=== abort because security error
		}
	} catch(...) { _warn("Can not check cap"); throw ; }

	try {
		if (do_we_need_to_change_uid_or_gid()) {
			_erro("We still have some UID/GID as root - when we expected to have none by now. Aborting.");
			std::abort(); // <=== abort because security error
		}
	} catch(...) { _warn("Can not check uid/gid"); throw ; }

	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
	}
	catch(const std::exception & ex) {
		_erro("Error: " << ex.what());
		throw ;
	}
	catch(...) {
		_erro("Error (unknown type)");
		throw ;
	}
	_fact("Security: privilages seem correct");
}


void drop_root(bool home_always_env) {
	t_changes_from_sudo changes;
	#ifdef ANTINET_linux
	try {
		changes = security_drop_root_from_sudo();
	} catch (const std::system_error &) {
		throw;
	} catch (const std::runtime_error &e) {
		_erro("Can not drop privileges to a regular user. We suggest to instead from a regular user call our program with sudo, and not run it directly as root");
		_erro(e.what());
	}
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
	if (home_always_env) {
		datastore::set_home(std::string());
	} else {
		datastore::set_home(changes.m_home_dir);
	}
}

}





