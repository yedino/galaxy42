
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


#ifdef ANTINET_linux
	#include <linux/types.h>
	#include <pwd.h>
	#include <unistd.h>

	// uses libcap-ng
	#include <cap-ng.h>

	#include "utils/capmodpp.hpp"
#endif


namespace my_cap {

void security_apply_cap_change(const capmodpp::cap_statechange_full & change) {
	#ifdef ANTINET_linux
	_note("Caps (before): " << capmodpp::read_process_caps() );
	_clue("Caps (will apply): " << change);
	change.security_apply_now();
	_note("Caps (after):  " << capmodpp::read_process_caps() );
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}

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

void security_drop_root_from_sudo() {
	_fact("Dropping root (if we are root)");
	#ifdef ANTINET_linux

	if ( ! do_we_need_to_change_uid_or_gid()) { _note("We are not root anyway"); return; } // not root

	_clue("Yes, will change UID/GID");
	_note("Caps (when changing user): " << capmodpp::read_process_caps() );

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

	capng_clear(CAPNG_SELECT_BOTH); // ***

	int ret = capng_update(CAPNG_ADD, static_cast<capng_type_t>(CAPNG_EFFECTIVE|CAPNG_PERMITTED), CAP_CHOWN);
	if (ret == -1) {
		_erro("capng_update error: " << ret);
		throw std::system_error(std::error_code(), "capng_update error, return value: " + std::to_string(ret));
	}
	ret = capng_change_id(normal_user_uid, normal_user_gid, static_cast<capng_flags_t>(CAPNG_DROP_SUPP_GRP | CAPNG_CLEAR_BOUNDING));
	if (ret != 0) {
		_erro("capng_change_id error: " << ret);
		throw std::system_error(std::error_code(), "capng_change_id error, return value: " + std::to_string(ret));
	}
	_fact("UID/GID change done");

/*	_fact("change process user id to " << normal_user_uid);
	int setuid_ret = setuid(normal_user_uid);
	if (setuid_ret == -1) {
		_fact("setuid error");
		throw std::system_error(std::error_code(errno, std::system_category()));
	}*/
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif

	if (do_we_need_to_change_uid_or_gid()) {
		_erro("Something is wrong, we tried to remove root UID/GID but stil it is not done. Aborting.");
		std::abort();
	}
}

void drop_privileges_on_startup() {
	#ifdef ANTINET_linux
	_fact("Dropping privileges - on startup");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_RAW", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used
	change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used

	if (do_we_need_to_change_uid_or_gid()) {
		// to drop root. is this really needed like that? needs more review.
		_fact("Leaving SETUID/SETGID (and PCAP) caps, to allow droping root UID later");
		change.set_given_cap("SETUID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("SETGID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("SETPCAP", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
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
	change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});

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
	#ifdef ANTINET_linux
	_info("Verifying privileges are dropped");

	bool have_any_cap = (capmodpp::secure_capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE);
	if (have_any_cap) {
		_erro("We still have some CAP capability, when we expected to have none by now. Aborting.");
		std::abort(); // <=== abort because security error
	}
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}


void drop_root() {
	#ifdef ANTINET_linux
	try {
		security_drop_root_from_sudo();
	} catch (const std::system_error &) {
		throw;
	} catch (const std::runtime_error &e) {
		_erro("Can not drop privileges to a regular user. We suggest to instead from a regular user call our program with sudo, and not run it directly as root");
		_erro(e.what());
	}
	#else
		_note("This security operation is not available on this system, ignoring");
	#endif
}

}





