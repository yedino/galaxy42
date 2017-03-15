// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "libs0.hpp"
#include "gtest/gtest_prod.h"

class c_galaxysrv_peers {
	protected:
		c_galaxysrv_peers()=default;
		virtual ~c_galaxysrv_peers()=default;

	public:
		void add_peer_simplestring(const string & simple); ///< add peer with given reference @see help_peer_ref()

		void help_peer_ref(ostream & ostr); ///< see function body for documentation too! Displays help: peer reference formats

		/// partially parsed reference. first is 0...1 elements the ID (HIP), and second is 0...N of cable reference
		using t_peering_reference_parse = pair<vector<string> , vector<string>>;

	protected:
		t_peering_reference_parse parse_peer_reference(const string & simple) const;
		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_test);
		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_throw_exceptions_test);
};

// -------------------------------------------------------------------


