
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

static bool do_we_need_to_change_uid_or_gid() {
	uid_t uid = getuid();
	if (uid == 0) return true;
	return false;
}

#ifdef ANTINET_linux
static void change_user_if_root() {
	_fact("Dropping root (if needed)");
	_note("Caps (in change user): " << capmodpp::read_process_caps() );

	uid_t uid = getuid();
	if (uid != 0) { _note("We are not root anyway"); return; } // not root

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

/*	_fact("change process user id to " << normal_user_uid);
	int setuid_ret = setuid(normal_user_uid);
	if (setuid_ret == -1) {
		_fact("setuid error");
		throw std::system_error(std::error_code(errno, std::system_category()));
	}*/
}

void drop_privileges_on_startup() {
	_fact("Dropping privileges - on startup");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_RAW", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used
	change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used

	if (do_we_need_to_change_uid_or_gid()) {
		// to drop root. is this really needed like that? needs more review.
		_fact("Leaving SETUID/SETGID caps, to allow droping root UID later");
		change.set_given_cap("SETUID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
		change.set_given_cap("SETGID", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	}

	_note("Caps (before): " << capmodpp::read_process_caps() );
	change.security_apply_now(); // TODO: must be commented, otherwise program works only with sudo called by root, this function must be called always after change_user_if_root()
	// XXX SECURITY - this was uncommented (by @rob) - @rfree
	_note("Caps (after):  " << capmodpp::read_process_caps() );
}


void drop_privileges_after_tuntap() {
	_fact("Dropping privileges - after tuntap");
	_note("Caps (before): " << capmodpp::read_process_caps() );
	try {
		change_user_if_root();
	} catch (const std::system_error &) {
		throw;
	} catch (const std::runtime_error &e) {
		_warn("Can not drop privileges to a regular user. We suggest to instead from a regular user call our program with sudo, and not run it directly as root");
		_warn(e.what());
	}

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.security_apply_now();

	_note("Caps (after):  " << capmodpp::read_process_caps() );
}

void drop_privileges_before_mainloop() {
	_fact("Dropping privileges - before mainloop");
	_note("Caps (before): " << capmodpp::read_process_caps() );
	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.security_apply_now();
	_note("Caps (after):  " << capmodpp::read_process_caps() );
}

void verify_privileges_are_as_for_mainloop() {
	_info("Verifying privileges are dropped");

	// using directly libcap-ng to confirm
	auto doit = []() { // doing work in thread, see notes of this file
		// based on https://people.redhat.com/sgrubb/libcap-ng/
		bool have_any_cap = (capmodpp::secure_capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE);
		if (have_any_cap) {
			_erro("We still have some CAP capability, when we expected to have none by now!");
			std::abort(); // <=== abort because security error
		}
	};
	doit();
}


#else

void drop_privileges_on_startup() {
}

void drop_privileges_after_tuntap() {
}

void drop_privileges_before_mainloop() {
}

void verify_privileges_are_as_for_mainloop() {
}

#endif

}





