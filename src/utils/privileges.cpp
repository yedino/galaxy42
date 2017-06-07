
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

	// uses libcap-ng
	#include <cap-ng.h>
#else
	#error "not supported"
#endif

#include "utils/capmodpp.hpp"

namespace my_cap {

#ifdef ANTINET_linux

void drop_privileges_after_tuntap() {
	_fact("Dropping privileges - after tuntap");
	auto doit = []() { // doing work in thread, see notes of this file
		// based on https://people.redhat.com/sgrubb/libcap-ng/
		capng_clear(CAPNG_SELECT_BOTH);
		capng_apply(CAPNG_SELECT_BOTH);
	};
	doit();
}

void drop_privileges_after_root() {
	_fact("Dropping privileges - after root");

	auto doit = []() { // doing work in thread, see notes of this file
		// based on https://people.redhat.com/sgrubb/libcap-ng/
		capng_clear(CAPNG_SELECT_BOTH);
		capng_apply(CAPNG_SELECT_BOTH);
	};
	doit();
}

void drop_privileges_before_mainloop() {
	_fact("Dropping privileges - before mainloop");

	auto doit = []() { // doing work in thread, see notes of this file
		// based on https://people.redhat.com/sgrubb/libcap-ng/
		capng_clear(CAPNG_SELECT_BOTH);
		capng_apply(CAPNG_SELECT_BOTH);
	};
	doit();
}

void verify_privileges_are_as_for_mainloop() {
	_info("Verifying privileges are dropped");

	auto doit = []() { // doing work in thread, see notes of this file
		// based on https://people.redhat.com/sgrubb/libcap-ng/
		bool have_any_cap = (capng_have_capabilities(CAPNG_SELECT_CAPS) > CAPNG_NONE);
		if (have_any_cap) {
			_erro("We still have some CAP capability, when we expected to have none by now!");
			std::abort(); // <=== abort because security error
		}
	};
	doit();
}

#else

void drop_privileges_after_tuntap() {
}
void drop_privileges_after_root() {
}
void drop_privileges_before_mainloop() {
}
void verify_privileges_are_as_for_mainloop() {
}

#endif

};





