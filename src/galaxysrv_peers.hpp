// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "libs0.hpp"
#include "gtest/gtest_prod.h"

#include "haship.hpp"
#include "cable/cards.hpp"
#include "cable/base/cable_base_addr.hpp"

/// The "static" reference to a peer, not so much the run-time state of it.
/// The #hip is my ID, or it can also be empty (hip.is_empty()) to rpresent that my HIP is not known yet....
/// ...the not known HIP is for --peer anyone@(udp:p.meshnet.pl:9042)
struct t_peer_reference_newloop {
	c_haship_addr hip; ///< identity of this peer as his HIP
	vector< unique_ptr<c_cable_base_addr> > cable_addr; ///< known cable-addresses (transport addresses) to this peer
	map<string,boost::any> options; ///< map of options for this peer, e.g. "score" => int(100)
	void print(ostream &ostr) const;
};
ostream& operator<<(ostream &ostr, const t_peer_reference_newloop & v);

/// The "runtime" state of peer to which I could be connected/connecting
/// The identity (HIP) of if can be empty, this means unknown (anyone@cable) HIP.
class c_peer_connection {
	public:
		t_peer_reference_newloop m_reference; ///< address informations with HIP and list of cable addresses

		c_peer_connection( t_peer_reference_newloop && ref ); ///< create connection for this reference (hip,cables)

		bool is_connected() const; ///< are we connected to it right now? (>=1 cable connected)
		bool should_connect() const; ///< should we try to connect to it as soon as possible? E.g. some hello_loop
		void print(ostream &ostr) const;
};
ostream& operator<<(ostream &ostr, const c_peer_connection & v);

class c_galaxysrv_peers {
	protected:
		c_galaxysrv_peers()=default;
		virtual ~c_galaxysrv_peers()=default;

	public:
		/// add peer with given reference in format as #help_peer_ref()
		void add_peer_simplestring(const string & simple);

		/// add peer from parsed reference.
		void add_peer(unique_ptr<t_peer_reference_newloop> && ref);

		void help_peer_ref(ostream & ostr); ///< see function body for documentation too! Displays help: peer reference formats

		/// partially parsed reference. first is 0...1 elements the ID (HIP), and second is 0...N of cable reference
		using t_peering_reference_parse = pair<vector<string> , vector<string>>;

	protected:
		t_peering_reference_parse parse_peer_reference(const string & simple) const;

		vector<unique_ptr<c_peer_connection>> m_peer; ///< my peers (connected or not), including unknown yet peers "anyone@cable"
		c_cable_cards m_cable_cards; ///< my cards to use the cables (transports), this objects represent my virtual transport network "card"

		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_test);
		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_throw_exceptions_test);
};

// -------------------------------------------------------------------


