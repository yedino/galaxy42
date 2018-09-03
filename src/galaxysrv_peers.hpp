// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

#include "gtest/gtest_prod.h"

#include "haship.hpp"
#include "cable/cards.hpp"
#include "cable/base/cable_base_addr.hpp"

#include <stdplus/with_mutex.hpp>

#include "gtest/gtest_prod.h"

/** This represents a reference to peer (also knownas "peer invite")
 * It is mostly the "static" reference to a peer (without some run-time information).
 * The #hip is my ID, or it can also be empty (hip.is_empty()) to represent that my HIP is not known yet (reference
 * to physical addr) ... the not known HIP is for --peer anyone@(udp:p.meshnet.pl:9042)
 * @loopver newloop
 */
struct t_peer_reference_newloop {
	c_haship_addr hip; ///< identity of this peer as his HIP
	std::vector< std::unique_ptr<c_cable_base_addr> > cable_addr; ///< known cable-addresses (transport addresses) to this peer
	std::map<std::string, boost::any> options; ///< map of options for this peer, e.g. "score" => int(100)
	void print(std::ostream &ostr) const;
};
std::ostream& operator<<(std::ostream &ostr, const t_peer_reference_newloop & v);

/**
 * The "runtime" state of peer to which I could be connected/connecting
 * The identity (HIP) of if can be empty, this means unknown (anyone@cable) HIP.
 */
class c_peer_connection {
	public:
		t_peer_reference_newloop m_reference; ///< address informations with HIP and list of cable addresses

		c_peer_connection( t_peer_reference_newloop && ref ); ///< create connection for this reference (hip,cables)

		bool is_connected() const; ///< are we connected to it right now? (>=1 cable connected)
		bool should_connect() const; ///< should we try to connect to it as soon as possible? E.g. some hello_loop
		void print(std::ostream &ostr) const;
};
std::ostream& operator<<(std::ostream &ostr, const c_peer_connection & v);

class c_galaxysrv_peers {
	FRIEND_TEST(galaxysrv_peers, parse_peer_reference_demotest);
	FRIEND_TEST(galaxysrv_peers, parse_peer_reference_test_easy);
	FRIEND_TEST(galaxysrv_peers, parse_peer_reference_test_incorrect);
	FRIEND_TEST(galaxysrv_peers, parse_peer_reference_throw_exceptions_test_old);

	protected:
		c_galaxysrv_peers()=default;
		virtual ~c_galaxysrv_peers()=default;

	public:
		/// add peer with given reference in format as #help_peer_ref()
		void add_peer_simplestring(const std::string & simple);

		/// add peer from parsed reference.
		void add_peer(std::unique_ptr<t_peer_reference_newloop> && ref);

		static void help_peer_ref(std::ostream & ostr); ///< see function body for documentation too! Displays help: peer reference formats

		/// partially parsed reference. first is 0...1 elements the ID (HIP), and second is 0...N of cable reference
		using t_peering_reference_parse = std::pair<std::vector<std::string> , std::vector<std::string>>;

		/* Parses the string as specified in help_peer_ref() into format described in t_peering_reference_parse see it;
		 * Includes parsing 'anyone@' token and other possible magical tokens if any (in future).
		 */
		static t_peering_reference_parse parse_peer_reference(const std::string & simple);

		/// Parses the string as specified in help_peer_ref(). This is the full parse (not just splitting string)
		static std::unique_ptr<t_peer_reference_newloop> parse_peer_simplestring(const std::string & simple);

	protected:

		std::vector<std::unique_ptr<c_peer_connection>> m_peer; ///< my peers (connected or not), including unknown yet peers "anyone@cable"

		stdplus::with_mutex<MutexShared,c_cable_cards> m_cable_cards; ///< my cards to use the cables (transports), this objects represent my virtual transport network "card"

		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_test);
		FRIEND_TEST(galaxysrv_peers, parse_peer_reference_throw_exceptions_test);
};

// -------------------------------------------------------------------


