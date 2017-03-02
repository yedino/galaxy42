
// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "galaxysrv.hpp"
#include "galaxysrv_peers.hpp"
#include "galaxysrv_cables.hpp"
#include "galaxysrv_p2p.hpp"
#include "galaxysrv_e2e.hpp"

// gal.peers.peers_add();

class c_galaxysrv {
	public:
		c_galaxysrv()=default;
		virtual ~c_galaxysrv()=default;

	public:
		c_galaxysrv_p2p p2p; ///< peer2peer tunnels
		c_galaxysrv_e2e e2e; ///< end2end tunnels
		c_galaxysrv_cables cables; ///< cable
		c_galaxysrv_peers peers; ///< peers
};

// -------------------------------------------------------------------

