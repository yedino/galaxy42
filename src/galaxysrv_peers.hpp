// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "libs0.hpp"

class c_galaxysrv_peers {
	protected:
		c_galaxysrv_peers()=default;
		virtual ~c_galaxysrv_peers()=default;

	public:
		void add_peer(const string & refer); ///< add peer with given reference; @see help_peer_ref() below for allowed formats

		void help_peer_ref(ostream & ostr); ///< see function body for documentation too! Displays help: peer reference formats
};

// -------------------------------------------------------------------


