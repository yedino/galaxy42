/**
Copyrighted (C) 2016, GPL v3 Licence (may include also other code)
See LICENCE.txt
*/

/**

GPL or other licence, see the licence file!

This is early pre-pre-alpha version, do NOT use it yet for anything,
do NOT run it other then in a test VM or better on isolated computer,
it has bugs and 'typpos'.
                                                 _       _
 _ __  _ __ ___       _ __  _ __ ___        __ _| |_ __ | |__   __ _
| '_ \| '__/ _ \_____| '_ \| '__/ _ \_____ / _` | | '_ \| '_ \ / _` |
| |_) | | |  __/_____| |_) | | |  __/_____| (_| | | |_) | | | | (_| |
| .__/|_|  \___|     | .__/|_|  \___|      \__,_|_| .__/|_| |_|\__,_|
|_|                  |_|                          |_|                
     _                       _                                    _   
  __| | ___      _ __   ___ | |_     _   _ ___  ___    _   _  ___| |_ 
 / _` |/ _ \    | '_ \ / _ \| __|   | | | / __|/ _ \  | | | |/ _ \ __|
| (_| | (_) |   | | | | (_) | |_    | |_| \__ \  __/  | |_| |  __/ |_ 
 \__,_|\___/    |_| |_|\___/ \__|    \__,_|___/\___|   \__, |\___|\__|
                                                       |___/          
 _                   _                     
| |__   __ _ ___    | |__  _   _  __ _ ___ 
| '_ \ / _` / __|   | '_ \| | | |/ _` / __|
| | | | (_| \__ \   | |_) | |_| | (_| \__ \
|_| |_|\__,_|___/   |_.__/ \__,_|\__, |___/
                                 |___/  

*/




/*

TODO(r) do not tunnel entire (encrypted) copy of TUN, trimm it from headers that we do not need
TODO(r) establish end-to-end AE (cryptosession)

TODO(r) - actually use IDe instead IDab for end2end
TOOD(r) - nonce controll? (authorize?) - to block replay attack

TODO(r) - separate search for pubkeys database

*/

/*

Use this tags in this project:
[confroute] - configuration, tweak - for the routing
[protocol] - code related to how exactly protocol (e.g. node2node) is defined

*/

/*

Current TODO / topics:
* routing with dijkstra
** re-routing data for someone else fails, probably because the data is not in TUN-format but it's just the datagram

*/


const char * g_the_disclaimer =
"*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere! ***"; // XXX

// The name of the hardcoded default demo that will be run with --devel (unless option --develdemo is given) can be set here:
const char * g_demoname_default = "route_dij";
// see function run_mode_developer() here to see list of possible values

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <streambuf>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <string.h>
#include <assert.h>

#include <boost/optional.hpp>

#include <thread>

#include <cstring>

#include <sodium.h>

#include "libs1.hpp"
#include "counter.hpp"
#include "cpputils.hpp"

// linux (and others?) select use:
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

// for low-level Linux-like systems TUN operations
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header
// #include <net/if_ether.h> // peer over eth later?
// #include <net/if_media.h> // ?

#include "cjdns-code/NetPlatform.h" // from cjdns


// #include <net/if_tap.h>
#include <linux/if_tun.h>
#include "c_json_load.hpp"
#include "c_ip46_addr.hpp"
#include "c_peering.hpp"
#include "generate_config.hpp"


#include "crypto/crypto.hpp" // for tests
#include "rpc/rpc.hpp"

#include "crypto-sodium/ecdh_ChaCha20_Poly1305.hpp"


#include "trivialserialize.hpp"
#include "galaxy_debug.hpp"

#include "glue_sodiumpp_crypto.hpp" // e.g. show_nice_nonce()

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------


namespace developer_tests {

bool wip_strings_encoding(boost::program_options::variables_map & argm) {
	UNUSED(argm);

	_mark("Tests of string encoding");
	_warn("Test needs rewritting");
	return false;
}

} // namespace

// ------------------------------------------------------------------

/***
  @brief interface for object that can act as p2p node
*/
class c_galaxy_node {
	public:
		c_galaxy_node()=default;
		virtual ~c_galaxy_node()=default;

		virtual void nodep2p_foreach_cmd( c_protocol::t_proto_cmd cmd, string_as_bin data )=0; ///< send given command/data to each peer

		///! return peering reference of a peer by given HIP. Will throw expected_not_found (read more)
		///! if require_pubkey, then will throw expected_not_found_missing_pubkey if peer is here but missing his pubkey
		virtual const c_peering & get_peer_with_hip( c_haship_addr addr , bool require_pubkey )=0;
};

// ------------------------------------------------------------------


// when we can not find e.g.a peer because we are missing his pubkey and it is required
class expected_not_found_missing_pubkey : public stdplus::expected_exception {
	public:
		const char* what() const noexcept override;
};

const char* expected_not_found_missing_pubkey::what() const noexcept {
		return "expected_not_found_missing_pubkey";
}



/***
@brief Use this to get information about route. It resp.: returns, stores and searches the information.
- m_search - pathes we now look for
- m_route_nexthop - known pathes
*/
class c_routing_manager { ///< holds knowledge about routes, and searches for new ones
	public: // TODO(r) make it private, when possible - e.g. when all operator<< are changed to public: print(ostream&) const;
		enum t_route_state { e_route_state_found, e_route_state_dead };

		enum t_search_mode {  // why we look for a route
			e_search_mode_route_own_packet, // we want ourselves to send there
			e_search_mode_route_other_packet,  // we want to route packet of someone else
			e_search_mode_help_find }; // some else is asking us about the route

		typedef	std::chrono::steady_clock::time_point t_route_time; ///< type for representing times using in routing search etc

		class c_route_info {
			public:
				t_route_state m_state; ///< e.g. e_route_state_found is route is ready to be used
				c_haship_addr m_nexthop; ///< hash-ip of next hop in this route
				c_haship_pubkey m_pubkey;

				int m_cost; ///< some general cost - currently e.g. in number of hops
				t_route_time m_time; ///< age of this route
				// int m_ttl; ///< at which TTL we got this reply

				c_route_info(c_haship_addr nexthop, int cost, const c_haship_pubkey & pubkey);

				int get_cost() const;
		};

		class c_route_reason {
			public:
				c_haship_addr m_his_addr; ///< his address to which we should tell him the path
				t_search_mode m_search_mode; ///< do we search it for him because we need to route for him, or because he asked, etc
				// c_haship_addr m_his_question; ///< the address about which we aksed

				c_route_reason(c_haship_addr his_addr, t_search_mode mode);

				bool operator<(const c_route_reason &other) const;
				bool operator==(const c_route_reason &other) const;
		};

		class c_route_reason_detail {
			public:
				t_route_time m_when; ///< when we hasked about this address last time
				int m_ttl; ///< with what TTL we are doing the search
				c_route_reason_detail( t_route_time when , int ttl );
		};

		class c_route_search {
			public:
				c_haship_addr m_addr; ///< goal of search: dst address
				bool m_ever; ///< was this ever actually searched yet
				t_route_time m_ask_time; ///< at which time we last time tried asking

				int m_ttl_used; ///< at which TTL we actually last time tried asking
				int m_ttl_should_use; ///< at which TTL we want to search, looking at our requests (this is optimization - it's same as highest value in m_requests[])
				// TODO(r)
				// guy ttl=4 --> ttl3 --> ttl2 --> ttl1 *MYSELF*, highest_ttl=1, when we execute then: send ttl=0, set ask_ttl=0
				// ... meanwhile ...
				//                    guy ttl4 --> ttl3 *MYSELF*, highest_ttl=3(!!!), when we execute then: send ttl=2 (when timeout!) then ask_ttl=2

				map< c_route_reason , c_route_reason_detail > m_request; ///< information about all other people who are asking about this address

				c_route_search(c_haship_addr addr, int basic_ttl);

				void add_request(c_routing_manager::c_route_reason reason, int ttl); ///< add info that this guy also wants to be informed about the path
				void execute( c_galaxy_node & galaxy_node );
		};



		// searches:
		typedef std::map< c_haship_addr, unique_ptr<c_route_search> > t_route_search_by_dst; ///< running searches, by the hash-ip of finall destination
		t_route_search_by_dst m_search; ///< running searches

		// known routes:
		typedef std::map< c_haship_addr, unique_ptr<c_route_info> > t_route_nexthop_by_dst; ///< routes to destinations: the hash-ip of next hop, by hash-ip of finall destination
		t_route_nexthop_by_dst m_route_nexthop; ///< known routes: the hash-ip of next hop, indexed by hash-ip of finall destination

		const c_route_info & add_route_info_and_return(c_haship_addr target, c_route_info route_info); ///< learn a route to this target. If it exists, then merge it correctly (e.g. pick better one)

	public:
		const c_route_info & get_route_or_maybe_search(c_galaxy_node & galaxy_node , c_haship_addr dst, c_routing_manager::c_route_reason reason, bool start_search, int search_ttl);
};

std::ostream & operator<<(std::ostream & ostr, std::chrono::steady_clock::time_point tp) {
	using namespace std::chrono;
	steady_clock::duration dtn = tp.time_since_epoch();
	return ostr << duration_cast<seconds>(dtn).count();
}

std::ostream & operator<<(std::ostream & ostr, const c_routing_manager::t_search_mode & obj) {
	switch (obj) {
		case c_routing_manager::e_search_mode_route_own_packet: return ostr<<"route_OWN";
		case c_routing_manager::e_search_mode_route_other_packet: return ostr<<"route_OTHER";
		case c_routing_manager::e_search_mode_help_find: return ostr<<"help_FIND";
	}
	_warn("Unknown reason"); return ostr<<"???";
}

std::ostream & operator<<(std::ostream & ostr, const c_routing_manager::c_route_info & obj) {
	return ostr << "{ROUTE: next_hop=" << obj.m_nexthop
		<< " cost=" << obj.m_cost << " time=" << obj.m_time << "}";
}

std::ostream & operator<<(std::ostream & ostr, const c_routing_manager::c_route_reason & obj) {
	return ostr << "{Reason: asked from " << obj.m_his_addr << " as " << obj.m_search_mode << "}";
}
std::ostream & operator<<(std::ostream & ostr, const c_routing_manager::c_route_reason_detail & obj) {
	return ostr << "{Reason...: at " << obj.m_when << " with TTL=" << obj.m_ttl << "}";
}

std::ostream & operator<<(std::ostream & ostr, const c_routing_manager::c_route_search & obj) {
	ostr << "{SEARCH for route to DST="<<obj.m_addr<<", was yet run=" << (obj.m_ever?"YES":"never")
		<< " ask: time="<<obj.m_ask_time<<" ttl should="<<obj.m_ttl_should_use << ", ttl used=" << obj.m_ttl_used;
	if (obj.m_request.size()) {
		ostr << "with " << obj.m_request.size() << " REQUESTS:" << endl;
		for(auto const & r : obj.m_request) ostr << " REQ: " << r.first << " => " << r.second << endl;
		ostr << endl;
	} else ostr << " (no requesters here)";
	ostr << "}";
	return ostr;
}


c_routing_manager::c_route_info::c_route_info(c_haship_addr nexthop, int cost, const c_haship_pubkey & pubkey)
	: m_state(e_route_state_found), m_nexthop(nexthop)
	, m_pubkey(pubkey)
	, m_cost(cost), m_time(  std::chrono::steady_clock::now() )
{ }

int c_routing_manager::c_route_info::get_cost() const { return m_cost; }

c_routing_manager::c_route_reason_detail::c_route_reason_detail( t_route_time when , int ttl )
	: m_when(when) , m_ttl ( ttl )
{ }

void c_routing_manager::c_route_search::add_request(c_routing_manager::c_route_reason reason , int ttl) {
	auto found = m_request.find( reason );
	if (found == m_request.end()) { // new reason for search
		c_route_reason_detail reason_detail( std::chrono::steady_clock::now() , ttl );
		_info("Adding new reason for search: " << reason << " details: " << reason_detail);
		m_request.emplace(reason, reason_detail);
	}
	else {
		auto & detail = found->second;
		_info("Updating reason of search: " << reason << " old detail: " << detail );
		detail.m_when = std::chrono::steady_clock::now();
		detail.m_ttl = std::max( detail.m_ttl , ttl ); // use the bigger TTL [confroute]
		_info("Updating reason of search: " << reason << " new detail: " << detail );
	}

	// update this search'es goal TTL
	// TODO(r)-refact: this could be factored into some generic: set_highest() , with optional debug too
	auto ttl_old = this->m_ttl_should_use;
	this->m_ttl_should_use = std::max( this->m_ttl_should_use , ttl);
	if (ttl_old != this->m_ttl_should_use) _info("Updated this search TTL to " << this->m_ttl_should_use << " from " << ttl_old);
}

c_routing_manager::c_route_reason::c_route_reason(c_haship_addr his_addr, t_search_mode mode)
	: m_his_addr(his_addr), m_search_mode(mode)
{
	_info("NEW reason: "<< (*this));
}

bool c_routing_manager::c_route_reason::operator<(const c_route_reason &other) const {
	if (this->m_his_addr < other.m_his_addr) return 1;
	if (this->m_search_mode < other.m_search_mode) return 1;
	return 0;
}

bool c_routing_manager::c_route_reason::operator==(const c_route_reason &other) const {
	return (this->m_his_addr == other.m_his_addr) && (this->m_search_mode == other.m_search_mode);
}

c_routing_manager::c_route_search::c_route_search(c_haship_addr addr, int basic_ttl)
	: m_addr(addr), m_ever(false), m_ask_time(), m_ttl_used(0), m_ttl_should_use(5)
{
	UNUSED(basic_ttl); // TODO or use it as m_ttl_should_use?
	_info("NEW router SEARCH: " << (*this));
}

const c_routing_manager::c_route_info & c_routing_manager::add_route_info_and_return(c_haship_addr target, c_route_info route_info) {
	// TODO(r): refactor out the create-or-update idiom
	auto it = m_route_nexthop.find( target );
	if (it == m_route_nexthop.end()) { // new one
		_info("This is NEW route information." << route_info);
		auto new_obj = make_unique<c_route_info>( route_info ); // TODO(rob): std::move it here - optimization?
		auto emplace = m_route_nexthop.emplace( std::move(target) , std::move(new_obj) );
		assert(emplace.second == true); // inserted new
		return * emplace.first->second; // reference to object stored in member we own
	} else {
		_info("This is UPDATED route information." << route_info);
		// TODO(r) TODONEXT pick optimal path?
		return * it->second;
	}
}

const c_routing_manager::c_route_info & c_routing_manager::get_route_or_maybe_search(c_galaxy_node & galaxy_node, c_haship_addr dst, c_routing_manager::c_route_reason reason, bool start_search , int search_ttl) {
	_info("ROUTING-MANAGER: find: " << dst << ", for reason: " << reason );

	try {
		const auto & peer = galaxy_node.get_peer_with_hip(dst,false); // no need for PK now, caller will do this on his own usually
		_info("We have that peer directly: " << peer );
		const int cost = 1; // direct peer. In future we can add connection cost or take into account congestion/lag...
		c_route_info route_info( peer.get_hip() , cost , * peer.get_pub() );
		_info("Direct route: " << route_info);
		const auto & route_info_ref_we_own = this -> add_route_info_and_return( dst , route_info ); // store it, so that we own this object
		return route_info_ref_we_own; // <--- return direct
	}
	catch(expected_not_found_missing_pubkey) { _dbg1("We LACK PUBLIC KEY for peer dst="<<dst<<" (but we have him besides that)"); } 
	catch(expected_not_found) { _dbg1("We do not have that dst="<<dst<<" in peers at all"); } // not found in direct peers

	auto found = m_route_nexthop.find( dst ); // <--- search what we know
	if (found != m_route_nexthop.end()) { // found
		const auto & route = found->second;
		_info("ROUTING-MANAGER: found route: " << (*route));
		return *route; // <--- warning: refrerence to this-owned object that is easily invalidatd
	}
	else { // don't have a planned route to him
		if (!start_search) {
			_info("No route, but we also so not want to search for it.");
			throw std::runtime_error("no route known (and we do NOT WANT TO search) to dst=" + STR(dst));
		}
		else {
			_info("Route not found, we will be searching");
			bool created_now=false;
			auto search_iter = m_search.find(dst);
			if (search_iter == m_search.end()) {
				created_now=true;
				_info("STARTED SEARCH (created brand new search record) for route to dst="<<dst);
				auto new_search = make_unique<c_route_search>(dst, search_ttl); // start a new search, at this TTL
				new_search->add_request( reason , search_ttl ); // add a first reason (it also sets TTL)
				auto search_emplace = m_search.emplace( std::move(dst) , std::move(new_search) );

				assert(search_emplace.second == true); // the insertion took place
				search_iter = search_emplace.first; // save here the result
			}
			else {
				_info("STARTED SEARCH (updated an existing search) for this to dst="<<dst);
				search_iter->second->add_request( reason , search_ttl ); // add reason (can increase TTL)
			}
			auto & search_obj = search_iter->second; // search exists now (new or updated)
			if (created_now) search_obj->execute( galaxy_node ); // ***
		}
	}
	_note("NO ROUTE");
	throw std::runtime_error("NO ROUTE known (at current time) to dst=" + STR(dst));
}

void  c_routing_manager::c_route_search::execute( c_galaxy_node & galaxy_node ) {
	_info("Sending QUERY for HIP, with m_ttl_should_use=" << m_ttl_should_use);
	string_as_bin data; // [protocol] for search query - format is: HIP_BINARY;TTL_BINARY;

	data += string_as_bin(m_addr);
	data += string(";");

	unsigned char byte_highest_ttl = m_ttl_should_use;  assert( m_ttl_should_use == byte_highest_ttl ); // TODO(r) asserted narrowing
	data += string(1, static_cast<char>(byte_highest_ttl) );
	data += string(";");

	galaxy_node.nodep2p_foreach_cmd( c_protocol::e_proto_cmd_findhip_query , data );

	m_ttl_used = byte_highest_ttl;
	m_ask_time = std::chrono::steady_clock::now();
}


// ------------------------------------------------------------------


class c_tunnel_use : public antinet_crypto::c_crypto_tunnel {
	public:
		int m_state; // s1..s4 (draft) TODO

	public:
		c_tunnel_use(const antinet_crypto::c_multikeys_PAIR & ID_self,
			const antinet_crypto::c_multikeys_pub & ID_them, const string& nicename);
		c_tunnel_use(const antinet_crypto::c_multikeys_PAIR & ID_self,
			const antinet_crypto::c_multikeys_pub & ID_them,
			const std::string & packetstart, const string& nicename );
};

c_tunnel_use::c_tunnel_use(const antinet_crypto::c_multikeys_PAIR & ID_self,
	const antinet_crypto::c_multikeys_pub & ID_them, const string& nicename)
	: c_crypto_tunnel(ID_self, ID_them, nicename)
{
}

c_tunnel_use::c_tunnel_use(const antinet_crypto::c_multikeys_PAIR & ID_self,
	const antinet_crypto::c_multikeys_pub & ID_them,
			const std::string & packetstart, const string& nicename )
	: c_crypto_tunnel(ID_self, ID_them, packetstart, nicename)
{
}



// ------------------------------------------------------------------

class c_tunserver : public c_galaxy_node {
	public:
		c_tunserver();

		void configure_mykey(); ///<  load my (this node's) keypair
		void run(); ///< run the main loop

		void set_my_name(const string & name); ///< set a nice name of this peer (shown in debug for example)

		void add_peer(const t_peering_reference & peer_ref); ///< add this as peer (just from reference)
		void add_peer_simplestring(const string & simple); ///< add this as peer, from a simple string like "ip-pub" TODO(r) instead move that to ctor of t_peering_reference
		///! add this user (or append existing user) with his actuall public key data
		void add_peer_append_pubkey(const t_peering_reference & peer_ref, unique_ptr<c_haship_pubkey> && pubkey);
		void add_tunnel_to_pubkey(const c_haship_pubkey & pubkey);


		void help_usage() const; ///< show help about usage of the program

		typedef enum {
			e_route_method_from_me=1, ///< I am the oryginal sender (try hard to send it)
			e_route_method_if_direct_peer=2, ///< Send data only if if I know the direct peer (e.g. I just route it for someone else - in star protocol the center node)
			e_route_method_default=3, ///< The default routing method
		} t_route_method;

		void nodep2p_foreach_cmd(c_protocol::t_proto_cmd cmd, string_as_bin data) override;
		const c_peering & get_peer_with_hip( c_haship_addr addr , bool require_pubkey ) override;

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop
		void wait_for_fd_event(); ///< waits for event of I/O being ready, needs valid m_tun_fd and others, saves the fd_set into m_fd_set_data

		std::pair<c_haship_addr,c_haship_addr> parse_tun_ip_src_dst(const char *buff, size_t buff_size, unsigned char ipv6_offset); ///< from buffer of TUN-format, with ipv6 bytes at ipv6_offset, extract ipv6 (hip) destination
		std::pair<c_haship_addr,c_haship_addr> parse_tun_ip_src_dst(const char *buff, size_t buff_size); ///< the same, but with ipv6_offset that matches our current TUN

		///@brief push the tunneled data to where they belong. On failure returns false or throws, true if ok.
		bool route_tun_data_to_its_destination_top(t_route_method method,
			const char *buff, size_t buff_size,
			c_haship_addr src_hip, c_haship_addr dst_hip,
			c_routing_manager::c_route_reason reason, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used);

		///@brief more advanced version for use in routing
		bool route_tun_data_to_its_destination_detail(t_route_method method,
			const char *buff, size_t buff_size,
			c_haship_addr src_hip, c_haship_addr dst_hip,
			c_haship_addr next_hip,
			c_routing_manager::c_route_reason reason,
			int recurse_level, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used);

		void peering_ping_all_peers();
		void debug_peers();

	private:
		string m_my_name; ///< a nice name, see set_my_name
		int m_tun_fd; ///< fd of TUN file
		unsigned char m_tun_header_offset_ipv6; ///< current offset in TUN/TAP data to the position of ipv6

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		typedef std::map< c_haship_addr, unique_ptr<c_peering> > t_peers_by_haship; ///< peers (we always know their IPv6 - we assume here), indexed by their hash-ip
		t_peers_by_haship m_peer; ///< my peers, indexed by their hash-ip

		t_peers_by_haship m_nodes; ///< all the nodes that I know about to some degree

		antinet_crypto::c_multikeys_PAIR m_my_IDC; ///< my keys!
		c_haship_addr m_my_hip; ///< my HIP that results from m_my_IDC, already cached in this format

		std::map< c_haship_addr, unique_ptr<c_tunnel_use> > m_tunnel; ///< my crypto tunnels

//		c_haship_pubkey m_haship_pubkey; ///< pubkey of my IP
//		c_haship_addr m_haship_addr; ///< my haship addres

		c_peering & find_peer_by_sender_peering_addr( c_ip46_addr ip ) const ;

		c_routing_manager m_routing_manager; ///< the routing engine used for most things
		/**
		 * @param ip_string contain ip address and port, i.e. 127.0.0.1:5000
		 * @retrun pair with ip string ad first and port as second
		 * @throw std::invalid_argument
		 * Exception safety: strong exception guarantee
		 */
		std::pair<string,int> parse_ip_string(const std::string &ip_string);
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

void c_tunserver::add_peer_simplestring(const string & simple) {
	_dbg1("Adding peer from simplestring=" << simple);
	// "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"
	size_t pos1 = simple.find('-');
	string part_pip = simple.substr(0,pos1);
	string part_hip = simple.substr(pos1+1);
	try {
		_info("Peer pip="<<part_pip<<" hip="<<part_hip);
		auto ip_pair = tunserver_utils::parse_ip_string(part_pip);
		_note("Physical IP: address=" << ip_pair.first << " port=" << ip_pair.second);
		this->add_peer( t_peering_reference( ip_pair.first, ip_pair.second , part_hip ) );
	}
	catch (const std::exception &e) {
		_erro("Adding peer from simplereference failed (exception): " << e.what()); // TODO throw?
	}
}

c_tunserver::c_tunserver()
 : m_my_name("unnamed-tunserver"), m_tun_fd(-1), m_tun_header_offset_ipv6(0), m_sock_udp(-1)
{
}

void c_tunserver::set_my_name(const string & name) {  m_my_name = name; _note("This node is now named: " << m_my_name);  }

// my key
void c_tunserver::configure_mykey() {
	// creating new IDC from existing IDI // this should be separated
	//and should include all chain IDP->IDM->IDI etc.  sign and verification

	// getting IDC
	std::string IDI_name = filestorage::load_string(e_filestore_galaxy_instalation_key_conf, "IDI");
	std::unique_ptr<antinet_crypto::c_multikeys_PAIR> my_IDI;
	my_IDI = std::make_unique<antinet_crypto::c_multikeys_PAIR>();
	my_IDI->datastore_load_PRV_and_pub(IDI_name);
	// getting HIP from IDI
	auto IDI_hexdot = my_IDI->get_ipv6_string_hexdot() ;
	c_haship_addr IDI_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDI_hexdot );
	_info("IDI IPv6: " << IDI_hexdot);
	_dbg1("IDI IPv6: " << IDI_hip << " (other var type)");
	// creating IDC for this session
	antinet_crypto::c_multikeys_PAIR my_IDC;
	my_IDC.generate(antinet_crypto::e_crypto_system_type_X25519,1);
	// signing it by IDI
	std::string IDC_pub_to_sign = my_IDC.m_pub.serialize_bin();
	antinet_crypto::c_multisign IDC_IDI_signature = my_IDI->multi_sign(IDC_pub_to_sign);

	// example veryifying
	antinet_crypto::c_multikeys_pub::multi_sign_verify(IDC_IDI_signature, IDC_pub_to_sign, my_IDI->m_pub);

	// remove IDP from RAM
	my_IDI.reset(nullptr);

	// for debug, hip from IDC
	auto IDC_hexdot = my_IDC.get_ipv6_string_hexdot() ;
	c_haship_addr IDC_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDC_hexdot );
	_info("IDC IPv6: " << IDC_hexdot);
	_dbg1("IDC IPv6: " << IDC_hip << " (other var type)");
	// now we can use hash ip from IDI and IDC for encryption
	m_my_hip = IDI_hip;
	m_my_IDC = my_IDC;

}

// add peer
void c_tunserver::add_peer(const t_peering_reference & peer_ref) { ///< add this as peer
	UNUSED(peer_ref);
	auto peering_ptr = make_unique<c_peering_udp>(peer_ref);
	// key is unique in map
	m_peer.emplace( std::make_pair( peer_ref.haship_addr ,  std::move(peering_ptr) ) );
}

void c_tunserver::add_peer_append_pubkey(const t_peering_reference & peer_ref,
unique_ptr<c_haship_pubkey> && pubkey)
{
	auto find = m_peer.find( peer_ref.haship_addr );
	if (find == m_peer.end()) { // no such peer yet
		auto peering_ptr = make_unique<c_peering_udp>(peer_ref);
		peering_ptr->set_pubkey(std::move(pubkey));
		m_peer.emplace( std::make_pair( peer_ref.haship_addr ,  std::move(peering_ptr) ) );
	} else { // update existing
		auto & peering_ptr = find->second;
		peering_ptr->set_pubkey(std::move(pubkey));
	}
}


void c_tunserver::add_tunnel_to_pubkey(const c_haship_pubkey & pubkey)
{
	_dbg1("add pubkey: " << pubkey.get_ipv6_string_hexdot());
	c_haship_addr hip( c_haship_addr::tag_constr_by_addr_bin() , pubkey.get_ipv6_string_bin() );

	auto find = m_tunnel.find(hip);
	if (find == m_tunnel.end()) { // we don't have tunnel to him yet
		_info("Creating a CT to HIP=" << hip);
		// TODO nicer name?
		auto ct = make_unique< c_tunnel_use >( m_my_IDC , pubkey , "Tunnel" );
		m_tunnel[ hip ] = std::move(ct);
	} else {
		_dbg2("Tunnel already is created for HIP="<<hip);
	}

}


void c_tunserver::help_usage() const {
	// TODO(r) remove, using boost options
}

void c_tunserver::prepare_socket() {
	m_tun_fd = open("/dev/net/tun", O_RDWR);
	assert(! (m_tun_fd<0) );

  as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);

	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, (void *)&ifr);
	m_tun_header_offset_ipv6 = g_tuntap::TUN_with_PI::header_position_of_ipv6; // matching the TUN/TAP type above
	if (errcode_ioctl < 0)_throw( std::runtime_error("Error in ioctl")); // TODO

	_mark("Allocated interface:" << ifr.ifr_name);

	{
		uint8_t address[16];
		assert(m_my_hip.size() == 16 && "m_my_hip != 16");
		for (int i=0; i<16; ++i) address[i] = m_my_hip[i];
		// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
		// ...to the device to which we are setting IP address here:
		assert(address[0] == 0xFD);
		assert(address[1] == 0x42);
		NetPlatform_addAddress(ifr.ifr_name, address, 16, Sockaddr_AF_INET6);
	}

	// create listening socket
	m_sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	_assert(m_sock_udp >= 0);

	int port = 9042;
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(port);

	{
		int bind_result = -1;
		if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
			sockaddr_in addr4 = address_for_sock.get_ip4();
			bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
		}
		else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
			sockaddr_in6 addr6 = address_for_sock.get_ip6();
			bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
		}
			_assert( bind_result >= 0 ); // TODO change to except
			_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
	}
	_info("Bind done - listening on UDP on: "); // TODO  << address_for_sock
}

void c_tunserver::wait_for_fd_event() { // wait for fd event
	_info("Selecting");
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_sock_udp, &m_fd_set_data);
	FD_SET(m_tun_fd, &m_fd_set_data);

	auto fd_max = std::max(m_tun_fd, m_sock_udp);
	_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	_assert(fd_max >= 1);

	timeval timeout { 3 , 0 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html

	auto select_result = select( fd_max+1, &m_fd_set_data, NULL, NULL, & timeout); // <--- blocks
	_assert(select_result >= 0);
}

std::pair<c_haship_addr,c_haship_addr> c_tunserver::parse_tun_ip_src_dst(const char *buff, size_t buff_size) { ///< the same, but with ipv6_offset that matches our current TUN
	return parse_tun_ip_src_dst(buff,buff_size, m_tun_header_offset_ipv6 );
}

std::pair<c_haship_addr,c_haship_addr> c_tunserver::parse_tun_ip_src_dst(const char *buff, size_t buff_size, unsigned char ipv6_offset) {
	// vuln-TODO(u) throw on invalid size + assert

	size_t pos_src = ipv6_offset + g_ipv6_rfc::header_position_of_src , len_src = g_ipv6_rfc::header_length_of_src;
	size_t pos_dst = ipv6_offset + g_ipv6_rfc::header_position_of_dst , len_dst = g_ipv6_rfc::header_length_of_dst;
	assert(buff_size > pos_src+len_src);
	assert(buff_size > pos_dst+len_dst);
	// valid: reading pos_src up to +len_src, and same for dst

	char ipv6_str[INET6_ADDRSTRLEN]; // for string e.g. "fd42:ffaa:..."

	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + pos_src, ipv6_str, INET6_ADDRSTRLEN); // ipv6 octets from 8 is source addr, from ipv6 RFC
	_dbg1("src ipv6_str " << ipv6_str);
	c_haship_addr ret_src(c_haship_addr::tag_constr_by_addr_dot(), ipv6_str);
	// TODONOW^ this works fine?

	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + pos_dst, ipv6_str, INET6_ADDRSTRLEN); // ipv6 octets from 24 is destination addr, from
	_dbg1("dst ipv6_str " << ipv6_str);
	c_haship_addr ret_dst(c_haship_addr::tag_constr_by_addr_dot(), ipv6_str);
	// TODONOW^ this works fine?

	return std::make_pair( ret_src , ret_dst );
}

void c_tunserver::peering_ping_all_peers() {
	auto & peers = m_peer;
	_info("Sending ping to all peers (count=" << peers.size() << ")");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived

		// [protocol] build raw
		trivialserialize::generator gen(8000);
		gen.push_varstring( m_my_IDC.get_serialize_bin_pubkey() );
		string_as_bin cmd_data( gen.str_move() );
		// TODONOW
		peer_udp->send_data_udp_cmd(c_protocol::e_proto_cmd_public_hi, cmd_data, m_sock_udp);
	}
}

void c_tunserver::nodep2p_foreach_cmd(c_protocol::t_proto_cmd cmd, string_as_bin data) {
	_info("Sending a COMMAND to peers:");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived
		peer_udp->send_data_udp_cmd(cmd, data, m_sock_udp);
	}
}

const c_peering & c_tunserver::get_peer_with_hip( c_haship_addr addr , bool require_pubkey ) {
	auto peer_iter = m_peer.find(addr);
	if (peer_iter == m_peer.end()) throw expected_not_found();
	c_peering & peer = * peer_iter->second;
	if (require_pubkey) {
		if (! peer.is_pubkey()) throw expected_not_found_missing_pubkey();
	}
	return peer;
}

void c_tunserver::debug_peers() {
	_note("=== Debug peers ===");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		_info("  * Known peer on key [ " << v.first << " ] => " << (* target_peer) );
	}
}

bool c_tunserver::route_tun_data_to_its_destination_detail(t_route_method method,
	const char *buff, size_t buff_size,
	c_haship_addr src_hip, c_haship_addr dst_hip,
	c_haship_addr next_hip,
	c_routing_manager::c_route_reason reason,
	int recurse_level, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used)
{
	// --- choose next hop in peering ---

	// try direct peers:
	auto peer_it = m_peer.find(next_hip); // find c_peering to send to // TODO(r) this functionallity will be soon doubled with the route search in m_routing_manager below, remove it then

	if (peer_it == m_peer.end()) { // not a direct peer!
		_info("ROUTE: can not find in direct peers next_hip="<<next_hip);
		if (recurse_level>1) {
			_warn("DROP: Recruse level too big in choosing peer");
			return false; // <---
		}

		c_haship_addr via_hip;
		try {
			_info("Trying to find a route to it");
			const int default_ttl = c_protocol::ttl_max_accepted; // for this case [confroute]
			const auto & route = m_routing_manager.get_route_or_maybe_search(*this, next_hip , reason , true, default_ttl);
			_info("Found route: " << route);
			via_hip = route.m_nexthop;
		} catch(...) { _info("ROUTE MANAGER: can not find route at all"); return false; }
		_info("Route found via hip: via_hip = " << via_hip);
		bool ok = this->route_tun_data_to_its_destination_detail(method, buff, buff_size,
			src_hip, dst_hip, via_hip, reason, recurse_level+1, data_route_ttl, nonce_used);
		if (!ok) { _info("Routing failed"); return false; } // <---
		_info("Routing seems to succeed");
	}
	else { // next_hip is a direct peer, send to it:
		auto & target_peer = peer_it->second;
		_info("ROUTE-PEER (found the goal in direct peer) selected peerig next hop is: " << (*target_peer) );
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived

		// send it on wire:
		peer_udp->send_data_udp(buff, buff_size, m_sock_udp, src_hip, dst_hip, data_route_ttl, nonce_used); // <--- *** actually send the data
	}
	return true;
}

bool c_tunserver::route_tun_data_to_its_destination_top(t_route_method method,
	const char *buff, size_t buff_size,
	c_haship_addr src_hip, c_haship_addr dst_hip,
	c_routing_manager::c_route_reason reason, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used) {
	try {
		_info("Sending data between end2end " << src_hip <<"--->" << dst_hip);
		bool ok = this->route_tun_data_to_its_destination_detail(method, buff, buff_size,
			src_hip, dst_hip, dst_hip, reason, 0, data_route_ttl, nonce_used);
		if (!ok) { _info("Routing/sending failed (top level)"); return false; }
	} catch(std::exception &e) {
		_warn("Can not send to peer, because:" << e.what()); // TODO more info (which peer, addr, number)
	} catch(...) {
		_warn("Can not send to peer (unknown)"); // TODO more info (which peer, addr, number)
	}
	_info("Routing/sending OK (top level)");
	return true;
}

c_peering & c_tunserver::find_peer_by_sender_peering_addr( c_ip46_addr ip ) const {
	for(auto & v : m_peer) { if (v.second->get_pip() == ip) return * v.second.get(); }
	throw std::runtime_error("We do not know a peer with such IP=" + STR(ip));
}

void c_tunserver::event_loop() {
	_info("Entering the event loop");
	c_counter counter(2,true);
	c_counter counter_big(10,false);

	this->peering_ping_all_peers();
	const auto ping_all_frequency = std::chrono::seconds( 3 ); // how often to ping them
	const auto ping_all_frequency_low = std::chrono::seconds( 1 ); // how often to ping first few times
	const long int ping_all_count_low = 2; // how many times send ping fast at first

	auto ping_all_time_last = std::chrono::steady_clock::now(); // last time we sent ping to all
	long int ping_all_count = 0; // how many times did we do that in fact


	// low level receive buffer
	const int buf_size=65536;
	char buf[buf_size];

	bool anything_happened=false; // in given loop iteration, for e.g. debug

	while (1) {
		// std::this_thread::sleep_for( std::chrono::milliseconds(100) ); // was needeed to avoid any self-DoS in case of TTL bugs

		auto time_now = std::chrono::steady_clock::now(); // time now

		{
			auto freq = ping_all_frequency;
			if (ping_all_count < ping_all_count_low) freq = ping_all_frequency_low;
			if (time_now > ping_all_time_last + freq ) {
				_note("It's time to ping all peers again (at auto-pinging time frequency=" << std::chrono::duration_cast<std::chrono::seconds>(freq).count() << " seconds)");
				peering_ping_all_peers(); // TODO(r) later ping only peers that need that
				ping_all_time_last = std::chrono::steady_clock::now();
				++ping_all_count;
			}
		}

		ostringstream oss;
		oss <<	" Node " << m_my_name << " hip=" << m_my_IDC.get_ipv6_string_hexdot() ;
		const string node_title_bar = oss.str();


		if (anything_happened || 1) {
			debug_peers();

			string xx(10,'-');
			_info('\n' << xx << node_title_bar << xx << "\n\n");
		} // --- print your name ---

		anything_happened=false;

		wait_for_fd_event();

		// TODO(r): program can be hanged/DoS with bad routing, no TTL field yet
		// ^--- or not fully checked. need scoring system anyway

		try { // ---

		if (FD_ISSET(m_tun_fd, &m_fd_set_data)) { // data incoming on TUN - send it out to peers
			anything_happened=true;

			auto size_read = read(m_tun_fd, buf, sizeof(buf)); // <-- read data from TUN
			_info("TTTTTTTTTTTTTTTTTTTTTTTTTT ###### ------> TUN read " << size_read << " bytes: [" << string(buf,size_read)<<"]");
			const int data_route_ttl = 5; // we want to ask others with this TTL to route data sent actually by our programs

			c_haship_addr src_hip, dst_hip;
			std::tie(src_hip, dst_hip) = parse_tun_ip_src_dst(buf, size_read);
			// TODO warn if src_hip is not our hip

			auto find_tunnel = m_tunnel.find( dst_hip ); // find end2end tunnel
			if (find_tunnel == m_tunnel.end()) {
				_warn("end2end tunnel does not exist, can not send OUR data from TUN to dst_hip="<<dst_hip);

				std::string dump; // just to trigger a search (for path - and btw for the pubkey!)
				_note("GET KEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY - will look for key for " << dst_hip << " so we can SEND THERE");
				this->route_tun_data_to_its_destination_top(
					e_route_method_from_me,
					dump.c_str(), dump.size(),
					src_hip, dst_hip,
					c_routing_manager::c_route_reason( c_haship_addr() , c_routing_manager::e_search_mode_route_own_packet),
					data_route_ttl
					,antinet_crypto::t_crypto_nonce()
				); // push the tunneled data to where they belong

			} else {
				_mark("Using CT tunnel to send our own data");
				auto & ct = * find_tunnel->second;
				antinet_crypto::t_crypto_nonce nonce_used;
				std::string data_cleartext( buf, buf+size_read);
				std::string data_encrypted = ct.box_ab(data_cleartext, nonce_used);

				this->route_tun_data_to_its_destination_top(
					e_route_method_from_me,
					data_encrypted.c_str(), data_encrypted.size(), // blob
					src_hip, dst_hip,
					c_routing_manager::c_route_reason( c_haship_addr() , c_routing_manager::e_search_mode_route_own_packet),
					data_route_ttl, nonce_used
				); // push the tunneled data to where they belong
			}
		}
		else if (FD_ISSET(m_sock_udp, &m_fd_set_data)) { // data incoming on peer (UDP) - will route it or send to our TUN
			anything_happened=true;

			sockaddr_in6 from_addr_raw; // peering address of peer (socket sender), raw format
			socklen_t from_addr_raw_size; // ^ size of it

			c_ip46_addr sender_pip; // peer-IP of peer who sent it

			// ***
			from_addr_raw_size = sizeof(from_addr_raw); // IN/OUT parameter to recvfrom, sending it for IN to be the address "buffer" size
			auto size_read = recvfrom(m_sock_udp, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
			_info("###### ======> UDP read " << size_read << " bytes: [" << string(buf,size_read)<<"]");
			// ^- reinterpret allowed by linux specs (TODO)
			// sockaddr *src_addr, socklen_t *addrlen);

			if (from_addr_raw_size == sizeof(sockaddr_in6)) { // the message arrive from IP pasted into sockaddr_in6 format
				_erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
				// trivial
			}
			else if (from_addr_raw_size == sizeof(sockaddr_in)) { // the message arrive from IP pasted into sockaddr_in (ipv4) format
				sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
				sender_pip.set_ip4(addr);
			} else {
				throw std::runtime_error("Data arrived from unknown socket address type");
			}

			_info("UDP Socket read from direct sender_pip = " << sender_pip <<", size " << size_read << " bytes: " << string_as_dbg( string_as_bin(buf,size_read)).get());
			// ------------------------------------

			// parse version and command:
			if (! (size_read >= 2) ) { _warn("INVALIDA DATA, size_read="<<size_read); continue; } // !
			assert( size_read >= 2 ); // buf: reads from position 0..1 are asserted as valid now

			int proto_version = static_cast<int>( static_cast<unsigned char>(buf[0]) ); // TODO
			_assert(proto_version >= c_protocol::current_version ); // let's assume we will be backward compatible (but this will be not the case untill official stable version probably)
			c_protocol::t_proto_cmd cmd = static_cast<c_protocol::t_proto_cmd>( buf[1] );

			// recognize the peering HIP/CA (cryptoauth is TODO)
			c_haship_addr sender_hip;
			c_peering * sender_as_peering_ptr  = nullptr; // TODO(r)-security review usage of this, and is it needed
			if (! c_protocol::command_is_valid_from_unknown_peer( cmd )) {
				c_peering & sender_as_peering = find_peer_by_sender_peering_addr( sender_pip ); // warn: returned value depends on m_peer[], do not invalidate that!!!
				_info("We recognize the sender, as: " << sender_as_peering);
				sender_hip = sender_as_peering.get_hip(); // this is not yet confirmed/authenticated(!)
				sender_as_peering_ptr = & sender_as_peering; // pointer to owned-by-us m_peer[] element. But can be invalidated, use with care! TODO(r) check this TODO(r) cast style
			}
			_info("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Command: " << cmd << " from peering ip = " << sender_pip << " -> peer HIP=" << sender_hip);

			if (cmd == c_protocol::e_proto_cmd_tunneled_data) { // [protocol] tunneled data
				_dbg1("Tunneled data");

				trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_buffer_valid() , buf, size_read );
				parser.skip_bytes_n(2);
				c_haship_addr src_hip(c_haship_addr::tag_constr_by_addr_bin() , parser.pop_bytes_n(g_ipv6_rfc::length_of_addr) );
				c_haship_addr dst_hip(c_haship_addr::tag_constr_by_addr_bin() , parser.pop_bytes_n(g_ipv6_rfc::length_of_addr) );
				int requested_ttl = parser.pop_byte_u(); // the TTL of data that we are asked to forward
				string nonce_used_raw = parser.pop_bytes_n( crypto_box_NONCEBYTES );
				_dbg1("nonce_used_raw="<<to_debug(nonce_used_raw));
				antinet_crypto::t_crypto_nonce nonce_used(
					sodiumpp::encoded_bytes(nonce_used_raw , sodiumpp::encoding::binary)
				);
				_warn("Received NONCE=" << antinet_crypto::show_nice_nonce(nonce_used) );
				string blob =	parser.pop_varstring(); // TODO view-string

/*
				std::unique_ptr<unsigned char []> decrypted_buf (new unsigned char[size_read + crypto_aead_chacha20poly1305_ABYTES]);
				unsigned long long decrypted_buf_len;

				int ttl_width=1; // the TTL heder width

				assert( size_read >= 1+2+ttl_width+1 );  // headers + anything

				assert(ttl_width==1); // we can "parse" just that now
				int requested_ttl = static_cast<char>(buf[1+2]); // the TTL of data that we are asked to forward

				assert(crypto_aead_chacha20poly1305_KEYBYTES <= crypto_generichash_BYTES);

				// reinterpret the char from IO as unsigned-char as wanted by crypto code
				unsigned char * ciphertext_buf = reinterpret_cast<unsigned char*>( buf ) + 2 + ttl_width; // TODO calculate depending on version, command, ...
				long long ciphertext_buf_len = size_read - 2 - 1; // TODO 2 = header size, and TTL
				assert( ciphertext_buf_len >= 1 );

				int r = crypto_aead_chacha20poly1305_decrypt(
					decrypted_buf.get(), & decrypted_buf_len,
					nullptr,
					ciphertext_buf, ciphertext_buf_len,
					additional_data, additional_data_len,
					nonce, generated_shared_key);
				if (r == -1) {
					_warn("Crypto verification failed!!!");
	//				continue; // skip this packet (main loop) // TODO
				}

				// TODO(r) factor out "reinterpret_cast<char*>(decrypted_buf.get()), decrypted_buf_len"

				// reinterpret for debug
				_info("UDP received, with cleartext:" << decrypted_buf_len << " bytes: [" << string( reinterpret_cast<char*>(decrypted_buf.get()), decrypted_buf_len)<<"]" );

				// can't wait till C++17 then with http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/p0144r0.pdf
				// auto { src_hip, dst_hip } = parse_tun_ip_src_dst(.....);
				c_haship_addr src_hip, dst_hip;
				std::tie(src_hip, dst_hip) = parse_tun_ip_src_dst(reinterpret_cast<char*>(decrypted_buf.get()), decrypted_buf_len);
*/

				// TODONOW optimize? make sure the proper binary format is cached:
				if (dst_hip == m_my_hip) { // received data addresses to us as finall destination:
					_info("UDP data is addressed to us as finall dst, sending it to TUN (after decryption) blob="<<to_debug(blob));

					auto find_tunnel = m_tunnel.find( src_hip ); // find end2end tunnel
					if (find_tunnel == m_tunnel.end()) {
						_warn("end2end tunnel does not exist, can not DECRYPT this data for us (yet?)...");

						std::string dump; // just to trigger a search (for path - and btw for the pubkey!)
						_note("GET KEYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY - will look for key for "
							<< dst_hip << " so we can READ DATA from there");
						this->route_tun_data_to_its_destination_top(
							e_route_method_from_me,
							dump.c_str(), dump.size(),
							dst_hip, src_hip, // return back to sender (from us)
							c_routing_manager::c_route_reason( c_haship_addr() , c_routing_manager::e_search_mode_route_own_packet),
							requested_ttl, // we assume sender is that far away from us, since the data reached us
							antinet_crypto::t_crypto_nonce() // any nonce - just dummy
						);

					} else {
						_mark("Using CT tunnel to decrypt data for us");
						auto & ct = * find_tunnel->second;
						auto tundata = ct.unbox_ab( blob , nonce_used );
						_note("<<<====== TUN INPUT: " << to_debug(tundata));
						ssize_t write_bytes = write(m_tun_fd, tundata.c_str(), tundata.size());
						if (write_bytes == -1) throw std::runtime_error("Fail to send UDP to TUN");
					} // we have CT
				}
				else
				{ // received data that is addresses to someone else
					auto data_route_ttl = requested_ttl - 1;
					const int limit_incoming_ttl = c_protocol::ttl_max_accepted;
					if (data_route_ttl > limit_incoming_ttl) {
						_info("We were requested to route (data) at high TTL (rude) by peer " << sender_hip <<  " - so reducing it.");
						data_route_ttl=limit_incoming_ttl;
					}

					_info("RRRRRRRRRRRRRRRRRRRRRRRRRRR UDP data is addressed to someone-else as finall dst, ROUTING it, at data_route_ttl="<<data_route_ttl);
					this->route_tun_data_to_its_destination_top(
						e_route_method_default,
						blob.c_str(), blob.size(),
						src_hip, dst_hip,
						c_routing_manager::c_route_reason( src_hip , c_routing_manager::e_search_mode_route_other_packet ),
						data_route_ttl,
						nonce_used // forward the nonce for blob
					); // push the tunneled data to where they belong // reinterpret char-signess
				}

			} // e_proto_cmd_tunneled_data
			else if (cmd == c_protocol::e_proto_cmd_public_hi) { // [protocol]
				_note("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh --> Command HI received");
				int offset1=2; assert( size_read >= offset1); // skip CMD headers (TODO instead use one parser)

				trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_buffer_valid() ,
					buf+offset1 , size_read-offset1);

				// TODONOW: size of pubkey is different, use serialize
				// if (cmd_data.bytes.at(pos1)!=';') throw std::runtime_error("Invalid protocol format, missing coma"); // [protocol]
				string_as_bin bin_his_pubkey( parser.pop_varstring() ); // PARSE
				_info("We received pubkey=" << to_debug( bin_his_pubkey ) );

				{ // add peer
					auto his_pubkey = make_unique<c_haship_pubkey>();
					his_pubkey->load_from_bin( bin_his_pubkey.bytes );
					_info("Parsed pubkey into: " << his_pubkey->to_debug());
					t_peering_reference his_ref( sender_pip , his_pubkey->get_ipv6_string_hexdot() );
					add_peer_append_pubkey( his_ref , std::move( his_pubkey ) );
				}

				{ // add node
					c_haship_pubkey his_pubkey;
					his_pubkey.load_from_bin( bin_his_pubkey.bytes );
					add_tunnel_to_pubkey( his_pubkey );
				}
			}
			else if (cmd == c_protocol::e_proto_cmd_findhip_query) { // [protocol]
				_warn("QQQQQQQQQQQQQQQQQQQQQQQ - we are QUERIED to find HIP");
				// [protocol] for search query - format is: HIP_BINARY;TTL_BINARY;
				int offset1=2; assert( size_read >= offset1);  string_as_bin cmd_data( buf+offset1 , size_read-offset1); // buf -> bin for comfortable use

				auto pos1 = cmd_data.bytes.find_first_of(';',offset1); // [protocol] size of HIP is dynamic  TODO(r)-ERROR XXX ';' is not escaped! will cause mistaken protocol errors
				decltype (pos1) size_hip = g_haship_addr_size; // possible size of HIP if ipv6
				if ((pos1==string::npos) || (pos1 != size_hip)) throw std::runtime_error("Invalid protocol format, wrong size of HIP field");

				string_as_bin bin_hip( cmd_data.bytes.substr(0,pos1) );
				c_haship_addr requested_hip( c_haship_addr::tag_constr_by_addr_bin(), bin_hip.bytes ); // *

				string_as_bin bin_ttl( cmd_data.bytes.substr(pos1+1,1) );
				int requested_ttl = static_cast<int>( bin_ttl.bytes.at(0) ); // char to integer

				auto data_route_ttl = requested_ttl - 1;
				const int limit_incoming_ttl = c_protocol::ttl_max_accepted;
				if (data_route_ttl > limit_incoming_ttl) {
					_info("We were requested to route (help search route) at high TTL (rude) by peer " << sender_hip <<  " - so reducing it.");
					data_route_ttl=limit_incoming_ttl;
                    UNUSED(data_route_ttl); // TODO is it should be used?
                }

				_info("We received request for HIP=" << string_as_hex( bin_hip ) << " = " << requested_hip << " and TTL=" << requested_ttl );
				if (requested_ttl < 1) {
					_info("Too low TTL, dropping the request");
				} else {
					c_routing_manager::c_route_reason reason( sender_hip , c_routing_manager::e_search_mode_help_find );
					try {
						_mark("Searching for the route he asks about");
						const auto & route = m_routing_manager.get_route_or_maybe_search(*this, requested_hip , reason , true, requested_ttl - 1);
						_note("We found the route thas he asks about, as: " << route);

						const int reply_ttl = requested_ttl; // will reply as much as needed

						// [protocol] e_proto_cmd_findhip_reply write "TTL;COST:HIP_OF_GOAL"
						trivialserialize::generator gen(50); // TODO optimal size
						gen.push_byte_u( reply_ttl );
						gen.push_byte_u( ';' );
						gen.push_byte_u( route.get_cost() );
						gen.push_byte_u( ';' );
						gen.push_bytes_n( g_haship_addr_size , string_as_bin( requested_hip ).bytes ); // the hip of goal
						gen.push_byte_u( ';' );
						gen.push_varstring( route.m_pubkey.serialize_bin() );
						gen.push_byte_u( ';' );

						auto data = gen.str();

						_info("DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD Will send data to sender_as_peering_ptr="
							<< sender_as_peering_ptr
							<< " data: " << to_debug_b( data ) );
						auto peer_udp = dynamic_cast<c_peering_udp*>( sender_as_peering_ptr ); // upcast to UDP peer derived
						peer_udp->send_data_udp_cmd(c_protocol::e_proto_cmd_findhip_reply, string_as_bin(data), m_sock_udp); // <---
						_note("Send the route reply");
					} catch(...) {
						_info("Can not yet reply to that route query.");
						// a background should be running in background usually
					}
				}

			}
			else if (cmd == c_protocol::e_proto_cmd_findhip_reply) { // [protocol]
				_warn("ROUTE GOT REPLY ggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg");
				// TODO-NOW format with hip etc
				// TODO-NOW here we will parse pubkey probably

				// [protocol] e_proto_cmd_findhip_reply read "TTL;COST:HIP_OF_GOAL"
				int offset1=2; // version, cmd
				trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_buffer_valid() ,  buf+offset1 , size_read-offset1);
				int given_ttl = parser.pop_byte_u(); // ttl
				parser.pop_byte_skip(';');
				int given_cost = parser.pop_byte_u(); // cost
				parser.pop_byte_skip(';');
				c_haship_addr given_goal_hip( c_haship_addr::tag_constr_by_addr_bin(),
					parser.pop_bytes_n( g_haship_addr_size ) ); // hip
				parser.pop_byte_skip(';');
				c_haship_pubkey pubkey; pubkey.load_from_bin( parser.pop_varstring() );
				parser.pop_byte_skip(';');
				_info("We have a TTL reply: ttl="<<given_ttl<<" goal="<<given_goal_hip<<" cost="<<given_cost);

				auto data_route_ttl = given_ttl - 1;
				const int limit_incoming_ttl = c_protocol::ttl_max_accepted;
				if (data_route_ttl > limit_incoming_ttl) {
					_info("Got command at high TTL (rude) by peer " << sender_hip <<  " - so reducing it.");
					data_route_ttl=limit_incoming_ttl;
				}

				if (given_ttl < 1) {
					_info("Too low TTL, dropping the request");
				} else {
					_info("GOT CORRECT REPLY - USING IT");

					_warn("Cool, we got there a pubkey.");
					add_tunnel_to_pubkey( pubkey );

					c_routing_manager::c_route_info route_info( sender_hip , given_cost , pubkey );
					_info("rrrrrrrrrrrrrrrrrrr route known thanks to peer help:" << route_info);
					// store it, so that we own this object:
					const auto & route_info_ref_we_own = m_routing_manager.add_route_info_and_return( given_goal_hip , route_info );
					UNUSED(route_info_ref_we_own); // TODO TODONOW and reply to others who asked us
				}
			}
			else {
				_warn("??????????????????? Unknown protocol command, cmd="<<cmd);
				continue; // skip this packet (main loop)
			}
			// ------------------------------------

		}
		else _info("Idle. " << node_title_bar);

		}
		catch (std::exception &e) {
			_warn("### !!! ### Parsing network data caused an exception: " << e.what());
		}

// stats-TODO(r) counters
//		int sent=0;
//		counter.tick(sent, std::cout);
//		counter_big.tick(sent, std::cout);
	}
}

void c_tunserver::run() {
	std::cout << "Stating the TUN router." << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------

namespace developer_tests {

string make_pubkey_for_peer_nr(int peer_nr) {
	string peer_pub = string("4a4b4c") + string("4") + string(1, char('0' + peer_nr)  );
	return peer_pub;
}

// list of peers that exist in our test-world:
struct t_peer_cmdline_ref {
	string ip;
	string pubkey;
	string privkey; ///< just in the test world. here we have knowledge of peer's secret key
};

bool wip_galaxy_route_star(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int node_nr = argm["develnum"].as<int>();  assert( (node_nr>=1) && (node_nr<=254) );
	std::cerr << "Running in developer mode - as node_nr=" << node_nr << std::endl;
	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");

	int peer_nr = node_nr==1 ? 2 : 1;

	string peer_pub = make_pubkey_for_peer_nr( peer_nr );
	// each connect to node .1., except the node 1 that connects to .2."
	string peer_ip = string("192.168.") + std::to_string( peer_nr  ) + string(".62");

	_mark("Developer: adding peer with arguments: ip=" << peer_ip << " pub=" << peer_pub );
	// argm.insert(std::make_pair("K", po::variable_value( int(node_nr) , false )));
	argm.insert(std::make_pair("peerip", po::variable_value( peer_ip , false )));
	argm.at("peerpub") = po::variable_value( peer_pub , false );
	argm.at("mypub") = po::variable_value( make_pubkey_for_peer_nr(node_nr)  , false );
	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(node_nr) , false );
	return true; // continue the test
}

void add_program_option_vector_strings(boost::program_options::variables_map & argm,
	const string & name , const string & value_to_append)
{
	namespace po = boost::program_options;
	vector<string> old_peer;
	try {
		old_peer = argm[name].as<vector<string>>();
		old_peer.push_back(value_to_append);
		argm.at(name) = po::variable_value( old_peer , false );
	} catch(boost::bad_any_cast) {
		old_peer.push_back(value_to_append);
		argm.insert( std::make_pair(name , po::variable_value( old_peer , false )) );
	}
	_info("program options: added to option '" << name << "' - now size: " << argm.at(name).as<vector<string>>().size() );
}

bool wip_galaxy_route_pair(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;

	if (my_nr == 1) add_program_option_vector_strings(argm, "peer", "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b");
	if (my_nr == 2) add_program_option_vector_strings(argm, "peer", "192.168.1.62:9042-fd42:ae11:f636:8636:ae76:acf5:e5c4:dae1");

	return true;
}

bool demo_sodiumpp_nonce_bug() {
	{
		_warn("test");

				string nonce_used_raw(24,0);
				nonce_used_raw.at(23)=6;

				_dbg1("nonce_used_raw="<<to_debug(nonce_used_raw));
				antinet_crypto::t_crypto_nonce nonce_used(
					sodiumpp::encoded_bytes(nonce_used_raw , sodiumpp::encoding::binary)
				);
				auto x = nonce_used;
				_warn("copy ok");
				auto y = nonce_used.get();
				_warn("get ok");

				_warn("Received NONCE=" << antinet_crypto::show_nice_nonce(nonce_used) );

				_warn("OK?");
				return false;
	}
}

bool wip_galaxy_route_doublestar(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;

	// --- define the test world ---
	// for given peer-number - the properties of said peer as seen by us (pubkey, ip - things given on the command line):
	map< int , std::string > peer_cmd_map = {
		{ 1 , "192.168.1.62:9042-fd42:9fd1:ce03:9edf:1d7e:2257:b651:d89f" } ,
		{ 2 , "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b" } ,
		{ 3 , "192.168.3.62:9042-fd42:5516:34c7:9302:890f:0a2d:5586:79ee" } ,
	};

	// list of connections in our test-world:
	map< int , vector<int> > peer_to_peer; // for given peer that we will create: list of his peer-number(s) that he peers into
	peer_to_peer[1] = vector<int>{ 2 , 3 };
	/*
	peer_to_peer[2] = vector<int>{ 4 , 5 };
	peer_to_peer[3] = vector<int>{ 6 , 7 };
	peer_to_peer[4] = vector<int>{ };
	peer_to_peer[5] = vector<int>{ };
	peer_to_peer[6] = vector<int>{ };
	peer_to_peer[7] = vector<int>{ };
	*/

	for (int peer_nr : peer_to_peer.at(my_nr)) { // for me, add the --peer refrence of all peers that I should peer into:
		_info("I connect into demo peer number: " << peer_nr);
		add_program_option_vector_strings(argm, "peer", peer_cmd_map.at(peer_nr));
	}

	_warn("Remember to set proper HOME with your key pair!");
	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(my_nr) , false );

/* TODO(r) bug#m153
	boost::any boostany = argm.at("peer");
	_erro("PEER = " << boostany);
	try { auto vvv = boost::any_cast<vector<string>>( argm.at("peer") ); }
	catch(...) { _warn("EXCEPT"); }
	_warn("after");
*/
	// _note("Done dev setup, runnig as: " << to_debug(argm));
	return true;
}



} // namespace developer_tests

/***
 * @brief Loads name of demo from demo.conf.
 * @TODO just loads file from current PWD, should instead load from program's starting pwd and also search user home dir.
 * @return Name of the demo to run, as configured in demo.conf -or- string "default" if can not load it.
*/
string demoname_load_conf(std::string democonf_fn = "config/demo.conf") {
	string ret="default";
	try {
		ifstream democonf_file(democonf_fn);
		if (! democonf_file.good()) { cerr<<"Not loading demo user config file ("<<democonf_fn<<")" << endl; return ret; }
		string line="";
		getline(democonf_file,line);
		if (! democonf_file.good()) { cerr<<"Failure in parsing demo user config file ("<<democonf_fn<<")" << endl; return ret; }
		ret = line.substr( string("demo=").size() );
	} catch(...) { }
	cerr<<"Loaded demo user config file ("<<democonf_fn<<") with demo option:" << ret << endl;
	return ret;
}


bool test_foo() {
	_info("TEST FOO");
	return false;
}

bool test_bar() {
	_info("TEST BAR");
	return false;
}


void test_lang_optional() {
	/* this fails on boost from debian 7
	boost::optional<float> x = boost::none;
	if (x) _info("too early: " << *x );
	x = 3.14;
	if (x) _info("and now: " << x.value() );
	*/
}

/***
@brief Run the main developer test in this code version (e.g. on this code branch / git branch)
@param argm - map with program options, it CAN BE MODIFIED here, e.g. the test can be to set some options and let the program continue
@return false if the program should quit after this test
*/
bool run_mode_developer_main(boost::program_options::variables_map & argm) {
	std::cerr << "Running in developer/demo mode." << std::endl;

	const string demoname_default = g_demoname_default;
	auto demoname = argm["develdemo"].as<string>();
	_note("Demoname (from program options command line) is:" << demoname);

	namespace poo = boost::program_options;
	poo::options_description desc("Possible demos");
	desc.add_options()
				//	("none", "no demo: start program in normal mode instead (e.g. to ignore demo config file)")
					("lang_optional", "foo boost::optional<>")
					("sodiumpp_bug", "sodiumpp nonce overflow constructor bug test (hits on older sodiumpp version)")
					("foo", "foo test")
					("bar", "bar test")
					("serialize",  "serialize test")
					("crypto", "crypto test")
					("gen_key_bench", "crypto benchmark")
					("crypto_stream_bench", "crypto stream benchmark")
					("ct_bench", "crypto tunel benchmark")
					("route_dij", "dijkstra test")
					("route", "current best routing (could be equal to some other test)")
					("debug", "some of the debug/logging functions")
					("rpc", "rpc demo")
					("help", "Help msg");

	if ((demoname=="help")||(demoname=="list")) {
		std::cout << "\nAvailable options for --demo NAME (or --devel --develdemo NAME) are following:";
		std::cout << desc << "\nChoose one of them as the NAME. But type it without the leading -- [TODO]" << std::endl; // TODO(janusz)
		return false;
	}

	const string demoname_loaded = demoname_load_conf();
	if (demoname_loaded != "default") demoname = demoname_loaded;
	if (demoname=="hardcoded") demoname = demoname_default;

	_note("Demo name selected: [" << demoname << "]");
	std::cout << std::string(70,'=')<<"\n" << "Demo: " << demoname << endl
		<< std::string(70,'=')<<"\n" << std::endl;

	if (demoname=="lang_optional") { test_lang_optional();  return false; }
	if (demoname=="foo") { test_foo();  return false; }
	if (demoname=="bar") { test_bar();  return false; }
	if (demoname=="serialize") { trivialserialize::test::test_trivialserialize();  return false; }
	if (demoname=="crypto") { antinet_crypto::test_crypto();  return false; }
	if (demoname=="gen_key_bench") { antinet_crypto::generate_keypairs_benchmark(2);  return false; }
	if (demoname=="crypto_stream_bench") { antinet_crypto::stream_encrypt_benchmark(2); return false; }
	if (demoname=="ct_bench") { antinet_crypto::multi_key_sign_generation_benchmark(2); return false; }
	if (demoname=="route_dij") { return developer_tests::wip_galaxy_route_doublestar(argm); }
	if (demoname=="route"    ) { return developer_tests::wip_galaxy_route_doublestar(argm); }
	if (demoname=="rpc") { rpc::rpc_demo(); return false; }
	if (demoname=="debug") { unittest::test_debug1(); return false; }


	_warn("Unknown Demo option ["<<demoname<<"] try giving other name, e.g. run program with --develdemo");
	return false;
}

bool run_mode_developer(boost::program_options::variables_map & argm) {
	auto ret = run_mode_developer_main(argm);
	std::cout << std::string(70,'=')<<"\n" << std::endl;
	return ret;
}

int main(int argc, char **argv) {
	std::cerr << std::endl << std::string(80,'=') << std::endl << g_the_disclaimer << std::endl << std::endl;

	g_dbg_level=20;

	{
		std::cerr<<"Starting lib sodium..."<<std::endl;
		ecdh_ChaCha20_Poly1305::init();
	}


/*	c_ip46_addr addr;
	std::cout << addr << std::endl;
	struct sockaddr_in sa;
	inet_pton(AF_INET, "192.0.2.33", &(sa.sin_addr));
	sa.sin_family = AF_INET;
	addr.set_ip4(sa);
	std::cout << addr << std::endl;

	struct sockaddr_in6 sa6;
	inet_pton(AF_INET6, "fc72:aa65:c5c2:4a2d:54e:7947:b671:e00c", &(sa6.sin6_addr));
	sa6.sin6_family = AF_INET6;
	addr.set_ip6(sa6);
	std::cout << addr << std::endl;
*/
	c_tunserver myserver;
	try {
		namespace po = boost::program_options;
		unsigned line_length = 120;
		po::options_description desc("Options", line_length);
		desc.add_options()
			("help", "Print help messages")

			("--debug", "-d")
			("-d", "Debug")

			("--quiet", "-q")
			("-q", "Quiet")

			("demo", po::value<std::string>()->default_value(""),
						"Try DEMO here. Run one of the compiled-in demonstrations of how program works.\n"
						"Use --demo help to see list of demos [TODO].")
			("devel","Test: used by developer to run current test")
			("develnum", po::value<int>()->default_value(1),
						"Test: used by developer to set current node number (makes sense with option --devel)")
			("develdemo", po::value<std::string>()->default_value("hardcoded"),
						"Test: used by developer to set current demo-test number/name(makes sense with option --devel)")
			// ("K", po::value<int>()->required(), "number that sets your virtual IP address for now, 0-255")
			("myname", po::value<std::string>()->default_value("galaxy") ,
						"a readable name of your node (e.g. for debug)")
			("gen-config", "Generate default .conf files:\n-galaxy.conf\n-connect_from.my.conf\n-connect_to.my.conf"
						   "\n-connect_to.seed.conf\n*** this could overwrite your actual configurations ***")

			("my-key", po::value<std::string>(), "Choose already generated key from default location")
			("my-key-file", po::value<std::string>(), "Choose key file from specified location")

			("info", "Print info about key specified in my-key option\nrequires [--my-key]")
			("list-my-keys", "List your key which are in default location")
			("set-IDI", "Set main instalation key (IDI) that will be use for signing connection (IDC) key"
						"\nrequires [--my-key]")

			("gen-key", "Generate combination of crypto key \nrequired [--new-key or --new-key-file, --key-type]"
						"\nexamples:"
						"\n--gen-key --new-key \"myself\" --key-type \"ed25519:x3\" \"rsa:x1:size=4096\""
						"\n--gen-key --new-key-file \"~/Documents/work/newkey.PRV\""
						"--key-type \"ed25519:x3\" \"rsa:x1:size=4096\"")
				("new-key", po::value<std::string>(), "Name of output key file in default location for keys")
				("new-key-file", po::value<std::string>(), "Name of output key file in specified location")
				("key-type", po::value<std::vector<std::string>>()->multitoken(), "Types of generated sub keys")

			("sign", "Sign key or other message with your key"
					 "\nrequires [--my-key, --my-key-file and sign-key sign-key-file\nexamples:"
					 "\n--sign --my-key \"myself\" --sign-key \"friend\""
					 "\n--sign --my-key-file \"/mount/usb2/work/work2\" --sign-data-file \"/mount/usb1/friend.public\"")
				("sign-key", po::value<std::string>(), "Name of key file in default location for keys")
				("sign-key-file", po::value<std::string>(), "Name of key file in specified location")
				("sign-data-file", po::value<std::string>(), "Name of data file in specified location")

			("verify", "Verify key or data with trusted-key and key or data"
					   "\nrequires [--trusted-key or --trusted-key-file and --toverify-key or --toverify-key-file "
					   "or --toverify-data-file *--signature-file]"
					   "\nDefault signature file name = key/data file name + \".sig\" extension"
					   "in same location as key/data file")
				("trusted-key", po::value<std::string>(), "Name of trusted key in default location")
				("trusted-key-file", po::value<std::string>(), "Name of trusted key file in specified location")
				("toverify-key", po::value<std::string>(), "Name of key to verify in default location")
				("toverify-key-file", po::value<std::string>(), "Name of key to verify file in specified location")
				("toverify-data-file", po::value<std::string>(), "Name of data file specified location")
				("signature-file", po::value<std::string>(),
							"External Name of signature file in specified location"
							"\nDefault signature file name = key/data file name + \".sig\" extension")

			("config", po::value<std::string>()->default_value("galaxy.conf") , "Load configuration file")
			("no-config", "Don't load any configuration file")

			("mypub", po::value<std::string>()->default_value("") , "your public key (give any string, not yet used)")
			("mypriv", po::value<std::string>()->default_value(""),
						"your PRIVATE key (give any string, not yet used - of course this is just for tests)")
			//("peerip", po::value<std::vector<std::string>>()->required(),
			//			"IP over existing networking to connect to your peer")
			//("peerpub", po::value<std::vector<std::string>>()->multitoken(), "public key of your peer")
			("peer", po::value<std::vector<std::string>>()->multitoken(),
						"Adding entire peer reference, in syntax like ip-pub."
						"Can be give more then once, for multiple peers.")
			;

		po::variables_map argm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), argm); // <-- parse actuall real command line options

			// === PECIAL options - that set up other program options ===

			{ // Convert shortcut options:  "--demo foo"   ----->   "--devel --develdemo foo"
				auto opt_demo = argm["demo"].as<string>();
				if ( opt_demo!="" ) {
					g_dbg_level_set(10,"Running in demo mode");
					_info("The demo command line option is given:" << opt_demo);
					// argm.insert(std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.at("develdemo") = po::variable_value( opt_demo , false );
					// (std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.insert(std::make_pair("devel",     po::variable_value( false , false ))); // --devel
				}
			}

			if (argm.count("devel")) { // can also set up additional options
				try {
					g_dbg_level_set(10,"Running in devel mode");
					_info("The devel mode is active");

					bool should_continue = run_mode_developer(argm);
					if (!should_continue) return 0;
				}
				catch(std::exception& e) {
					std::cerr << "Unhandled Exception reached the top of main: (in DEVELOPER MODE)" << e.what()
							  << ", application will now exit" << std::endl;
						return 0; // no error for developer mode
				}
			}

			// === argm now can contain options added/modified by developer mode ===
			po::notify(argm);  // !
			// --- debug level for main program ---
			g_dbg_level_set(20,"For normal program run");
			if (argm.count("--debug") || argm.count("-d")) g_dbg_level_set(10,"For debug program run");
			if (argm.count("--quiet") || argm.count("-q")) g_dbg_level_set(200,"For quiet program run");

			if (argm.count("help")) { // usage
				std::cout << desc;
				return 0;
			}

			if (argm.count("set-IDI")) {
				if (!argm.count("my-key")) {
					_erro("--my-key is required for --set-IDI");
					return 1;
				}
				auto name = argm["my-key"].as<std::string>();
				auto keys_path = filestorage::get_parent_path(e_filestore_galaxy_wallet_PRV,"");
				auto keys = filestorage::get_file_list(keys_path);
				bool found = false;
				for (auto &key_name : keys) {
					//remove .PRV extension
					size_t pos = key_name.find(".PRV");
					std::string act = key_name.substr(0,pos);
					if (name == act) {
						found = true;
						std::cout << "Found key: " << found << std::endl;
						break;
					}
				}
				if (found == false) {
					_erro("Can't find:" << name << " key in your key list");
					return 1;
				}
				filestorage::save_string(e_filestore_galaxy_instalation_key_conf,"IDI", name, true);

				return 0;
			}

			if (argm.count("info")) {
				if (!argm.count("my-key")) {
					_erro("--my-key is required for --info");
					return 1;
				}
				auto name = argm["my-key"].as<std::string>();
				antinet_crypto::c_multikeys_pub keys;
				keys.datastore_load(name);
				_info(keys.to_debug());
				return 0;
			}

			if (argm.count("list-my-keys")) {
				auto keys_path = filestorage::get_parent_path(e_filestore_galaxy_wallet_PRV,"");
				std::vector<std::string> keys = filestorage::get_file_list(keys_path);
				std::string IDI_key = "";
			try {
				IDI_key = filestorage::load_string(e_filestore_galaxy_instalation_key_conf, "IDI");
			} catch (std::invalid_argument &err) {
				_dbg2("IDI is not set!");
			}
				std::cout << "Your key list:" << std::endl;
				for(auto &key_name : keys) {
					//remove .PRV extension
					size_t pos = key_name.find(".PRV");
					std::string actual_key = key_name.substr(0,pos);
					std::cout << actual_key << (IDI_key == actual_key ? " * IDI" : "") << std::endl;
				}
				return 0;
			}

			if (argm.count("gen-key")) {
				if (!argm.count("key-type")) {
					_erro("--key-type option is required for --gen-key");
					return 1;
				}
				std::vector<std::pair<antinet_crypto::t_crypto_system_type,int>> keys;
				auto arguments = argm["key-type"].as<std::vector<std::string>>();
				for (auto argument : arguments) {
					_dbg1("parse argument " << argument);
					std::replace(argument.begin(), argument.end(), ':', ' ');
					std::istringstream iss(argument);
					std::string str;
					iss >> str;
					_dbg1("type = " << str);
					antinet_crypto::t_crypto_system_type type = antinet_crypto::t_crypto_system_type_from_string(str);
					iss >> str;
					assert(str[0] == 'x');
					str.erase(str.begin());
					int number_of_keys = std::stoi(str);
					_dbg1("number_of_keys" << number_of_keys);
					keys.emplace_back(std::make_pair(type, number_of_keys));
				}

				std::string output_file;
				if (argm.count("new-key")) {
					output_file = argm["new-key"].as<std::string>();
					generate_config::any_crypto_set(output_file, keys, true);
				} else if (argm.count("new-key-file")) {
					output_file = argm["new-key-file"].as<std::string>();
					generate_config::any_crypto_set(output_file, keys, false);
				} else {
					_erro("--new-key or --new-key-file option is required for --gen-key");
					return 1;
				}
				return 0;
			}

			if (argm.count("sign")) {

				antinet_crypto::c_multikeys_PRV signing_key;
				std::string output_file;
				if (argm.count("my-key")) {
					output_file = argm["my-key"].as<std::string>();
					signing_key.datastore_load(output_file);

				} else if (argm.count("my-key-file")) {
					output_file = argm["my-key-file"].as<std::string>();
					sodiumpp::locked_string key_data(filestorage::load_string_mlocked(e_filestore_local_path,
																					  output_file));
					signing_key.load_from_bin(key_data.get_string());

				} else {
					_erro("--my-key or --my-key-file option is required for --sign");
					return 1;
				}

				std::string to_sign_file;
				std::string to_sign;
				if (argm.count("sign-key")) {
					to_sign_file = argm["sign-key"].as<std::string>();
					to_sign = filestorage::load_string(e_filestore_galaxy_pub, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					// adding ".pub" to make signature.pub.sig it's more clear (key.pub.sig is signature of key.pub)
					filestorage::save_string(e_filestore_galaxy_sig, to_sign_file+".pub", sign.serialize_bin(), true);
				} else if (argm.count("sign-key-file")) {
					to_sign_file = argm["sign-key-file"].as<std::string>();
					to_sign = filestorage::load_string(e_filestore_local_path, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					filestorage::save_string(e_filestore_local_path, to_sign_file+".sig", sign.serialize_bin(), true);

				} else if (argm.count("sign-data-file")) {
					to_sign_file = argm["sign-data-file"].as<std::string>();
					to_sign = filestorage::load_string(e_filestore_local_path, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					filestorage::save_string(e_filestore_local_path, to_sign_file+".sig", sign.serialize_bin(), true);

				} else {
					_erro("-sign-key, sign-key-file or -sign-data-file option is required for --sign");
					return 1;
				}
				return 0;
			}

			if(argm.count("verify")) {

				antinet_crypto::c_multikeys_pub trusted_key;
				std::string output_file;
				if (argm.count("trusted-key")) {
					output_file = argm["trusted-key"].as<std::string>();
					trusted_key.datastore_load(output_file);

				} else if (argm.count("trusted-key-file")) {
					output_file = argm["trusted-key-file"].as<std::string>();
					std::string key_data(filestorage::load_string(e_filestore_local_path, output_file));
					trusted_key.load_from_bin(key_data);

				} else {
					_erro("--trusted-key or --trusted-key-file option is required for --verify");
					return 1;
				}

				bool extern_signature = false;
				antinet_crypto::c_multisign signature;

				// usefull local function
				auto load_signature = [&signature] (t_filestore stype, const std::string &filename) {
					std::string data (filestorage::load_string(stype, filename));
					signature.load_from_bin(data);
				};

				std::string signature_file;
				if (argm.count("signature-file")) {
					extern_signature = true;
					signature_file = argm["signature-file"].as<std::string>();
					load_signature(e_filestore_local_path, signature_file);
				}

				std::string to_verify_file;
				std::string to_verify;
				if (argm.count("toverify-key")) {
					to_verify_file = argm["toverify-key"].as<std::string>();
					to_verify = filestorage::load_string(e_filestore_galaxy_pub, to_verify_file);
					signature_file = to_verify_file + ".pub";
					if(!extern_signature)
						load_signature(e_filestore_galaxy_sig, signature_file);

				} else if (argm.count("toverify-key-file")) {
					to_verify_file = argm["toverify-key-file"].as<std::string>();
					to_verify = filestorage::load_string(e_filestore_local_path, to_verify_file);
					signature_file = to_verify_file+".sig";
					if(!extern_signature)
						load_signature(e_filestore_local_path, signature_file);

				} else if (argm.count("toverify-data-file")) {
					to_verify_file = argm["toverify-data-file"].as<std::string>();
					to_verify = filestorage::load_string(e_filestore_local_path, to_verify_file);
					signature_file = to_verify_file+".sig";
					if(!extern_signature)
						load_signature(e_filestore_local_path, signature_file);
				} else {
					_erro("-toverify-key, toverify-key-file or -sign-data-file option is required for --sign");
					return 1;
				}

			try {
				antinet_crypto::c_multikeys_pub::multi_sign_verify(signature,to_verify,trusted_key);
			} catch (std::invalid_argument &err) {
				_dbg2("Signature verification: fail");
				return 1;
			}
				_dbg2("Verify Success");
				return 0;
			}

			if (argm.count("gen-config")) {
				c_json_genconf::genconf();
			}

			if (!(argm.count("no-config"))) {
				// loading peers from configuration file (default from galaxy.conf)
				std::string conf = argm["config"].as<std::string>();
				c_galaxyconf_load galaxyconf(conf);
				for(auto &ref : galaxyconf.get_peer_references()) {
					myserver.add_peer(ref);
				}
			}

			_info("Configuring my own reference (keys):");
			myserver.configure_mykey();
			myserver.set_my_name( argm["myname"].as<string>() );

			_info("Configuring my peers references (keys):");
			vector<string> peers_cmdline;
			try { peers_cmdline = argm["peer"].as<vector<string>>(); } catch(...) { }
			for (const string & peer_ref : peers_cmdline) {
				myserver.add_peer_simplestring( peer_ref );
			}


		}
		catch(po::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return 1;
		}
	}
	catch(std::exception& e) {
		std::cerr << "Unhandled Exception reached the top of main: "
				  << e.what() << ", application will now exit" << std::endl;
		return 2;
	}

	myserver.run();
}


