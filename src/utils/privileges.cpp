
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

#ifdef ANTINET_linux
static void change_user_if_root() {
	uid_t uid = getuid();
	if (uid != 0) return; // not root
	const char* sudo_user_env = std::getenv("SUDO_USER");
	if (sudo_user_env == nullptr) throw std::runtime_error("Cannot read SUDO_USER env") ; // get env error
	std::string sudo_user_name(sudo_user_env);
	struct passwd *pw = getpwnam(sudo_user_name.c_str());
	if (pw == nullptr) throw std::runtime_error("getpwnam error");
	uid_t normal_user_uid = pw->pw_uid;
	assert(normal_user_uid != 0);
	_fact("change process user id to " << normal_user_uid);
	int setuid_ret = setuid(normal_user_uid);
	if (setuid_ret == -1) throw std::runtime_error("setuid error");
	assert(setuid_ret == 0);
}

void drop_privileges_on_startup() {
	_fact("Dropping privileges - on startup");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_RAW", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used
	change.set_given_cap("NET_BIND_SERVICE", {capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged}); // not yet used
	change.security_apply_now();
	change_user_if_root();
}


void drop_privileges_after_tuntap() {
	_fact("Dropping privileges - after tuntap");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_unchanged, capmodpp::v_permit_unchanged, capmodpp::v_inherit_unchanged});
	change.set_given_cap("NET_ADMIN", {capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.security_apply_now();
}

void drop_privileges_before_mainloop() {
	_fact("Dropping privileges - before mainloop");

	capmodpp::cap_statechange_full change;
	change.set_all_others({capmodpp::v_eff_disable, capmodpp::v_permit_disable, capmodpp::v_inherit_disable});
	change.security_apply_now();
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





