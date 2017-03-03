// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "libs0.hpp"

class c_galaxysrv_peers {
	protected:
		c_galaxysrv_peers()=default;
		virtual ~c_galaxysrv_peers()=default;

	public:
		void add_peer_simplestring(const string & simple); ///< add peer with given reference @see help_peer_ref()

		void help_peer_ref(ostream & ostr); ///< see function body for documentation too! Displays help: peer reference formats

		/// partially parsed reference. first is 0...1 elements the ID (HIP), and second is 0...N of cable reference
		typedef pair<vector<string> , vector<string>> t_peering_reference_parse;

	protected:
		t_peering_reference_parse parse_peer_reference(const string & simple) const;
};

// -------------------------------------------------------------------


