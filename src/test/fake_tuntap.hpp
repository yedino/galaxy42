#ifndef FAKE_TUNTAP_HPP
#define FAKE_TUNTAP_HPP

#include "gtest/gtest.h"

#include "../libs0.hpp"
/*
#include "../galaxysrv.hpp"
#include "../newloop.hpp"
#include "../cable/simulation/world.hpp"
#include "../cable/simulation/cable_simul_obj.hpp"
#include "../cable/simulation/cable_simul_addr.hpp"
*/
// ============================================================================

/// For tests this simulates the kernel that provides data that can be read via tuntap
class c_tuntap_fake_kernel {
	public:
		c_tuntap_fake_kernel();

		size_t readtun( char * buf , size_t bufsize ); ///< [thread_safe] semantics like "read" from C.

	protected:
		std::atomic<int> m_readtun_nr;
		std::vector<std::string> m_data; ///< example packet of data

		std::string make_example(int nr);
		int pseudo_rand(int r1, int r2, int r3=0);
};

// -------------

void thread_test();

// ============================================================================

class c_tuntap_fake {
	public:
		c_tuntap_fake( c_tuntap_fake_kernel & kernel );
		size_t readtun( char * buf , size_t bufsize ); ///< [thread_safe] semantics like "read" from C.
	protected:
		c_tuntap_fake_kernel & m_kernel;
};

// ============================================================================


#endif // FAKE_TUNTAP_HPP
