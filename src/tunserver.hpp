// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

// TODO reduce headers:

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <streambuf>

#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>

#include <boost/program_options.hpp>

#include "protocol.hpp"
#include "c_peering.hpp"
#include "generate_crypto.hpp"

#include "crypto/crypto.hpp" // for tests
#include "rpc/rpc.hpp"

#include "trivialserialize.hpp"
#include "galaxy_debug.hpp"

#include "glue_sodiumpp_crypto.hpp" // e.g. show_nice_nonce()

#include "ui.hpp"

#include "tunserver.hpp"
#include "c_tun_device.hpp"
#include "c_udp_wrapper.hpp"
#include "c_event_manager.hpp"
#include <ctime>
#ifdef HTTP_DBG
#include "httpdbg/httpdbg-server.hpp"
#endif

// ------------------------------------------------------------------

extern const char * g_the_disclaimer;
extern const char * g_demoname_default;

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

// ------------------------------------------------------------------

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

// ------------------------------------------------------------------

class c_tunserver : public c_galaxy_node {
	public:
		c_tunserver();
		void set_desc(shared_ptr< boost::program_options::options_description > desc);

		void configure_mykey(); ///<  load my (this node's) keypair
		void run(int time = 0); ///< run the main loop

		/// @name Functions that execute a program action like creation of key, calculating signature, etc.
		/// @{
		void program_action_set_IDI(const string & keyname); ///< set configured IDI key (write the config to disk)
		void program_action_gen_key(boost::program_options::variables_map & argm); ///< generate a key according to given options
		std::string program_action_gen_key_simple(); ///< generates recommended simple key, returns name e.g. "IDI"
		/// @}

		void set_my_name(const string & name); ///< set a nice name of this peer (shown in debug for example)
		const antinet_crypto::c_multikeys_pub & read_my_IDP_pub() const; ///< read the pubkey of the (main / permanent) ID of this server
		string get_my_ipv6_nice() const; ///< returns the main HIP IPv6 of this node in a nice format (e.g. hexdot)
		int get_my_stats_peers_known_count() const; ///< get the number of currently known peers, for information

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
		void event_loop(int time = 0); ///< the main loop
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
        #ifdef HTTP_DBG
		std::mutex & get_my_mutex() const; ///< [thread] get lock guard on this
		friend class c_httpdbg_raport; ///< this is authorized to read my data for debug. but [thread] lock access first!!!
        #endif
	private:
        #ifdef HTTP_DBG
		mutable std::mutex m_my_mutex; ///< [thread] lock this before woring on this class (to protect from access from e.g. httpdbg)
        #endif
		string m_my_name; ///< a nice name, see set_my_name
		//int m_tun_fd; ///< fd of TUN file
		#ifdef __linux__
		c_tun_device_linux m_tun_device;
		c_udp_wrapper_linux m_udp_device;
		c_event_manager_linux m_event_manager;
		#elif defined(_WIN32) || defined(__CYGWIN__)
		c_tun_device_windows m_tun_device;
		c_udp_wrapper_asio m_udp_device;
		c_event_manager_windows m_event_manager;
		#elif defined(__MACH__)
		c_tun_device_empty m_tun_device;		// c_tun_device_mach?
		c_udp_wrapper_asio m_udp_device;		// c_udp_wrapper_asio?
		c_event_manager_mach m_event_manager;	// c_event_manager_mach?
		#else
		c_tun_device_empty m_tun_device;
		c_udp_wrapper_empty m_udp_device;
		c_event_manager_empty m_event_manager;
		#endif
		unsigned char m_tun_header_offset_ipv6; ///< current offset in TUN/TAP data to the position of ipv6

		shared_ptr< boost::program_options::options_description > m_desc; ///< The boost program options that I will be using. (Needed for some internal commands)

//		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		typedef std::map< c_haship_addr, unique_ptr<c_peering> > t_peers_by_haship; ///< peers (we always know their IPv6 - we assume here), indexed by their hash-ip
		t_peers_by_haship m_peer; ///< my peers, indexed by their hash-ip

		t_peers_by_haship m_nodes; ///< all the nodes that I know about to some degree

		antinet_crypto::c_multikeys_PAIR m_my_IDC; ///< my keys!
		antinet_crypto::c_multikeys_pub	m_my_IDI_pub;	/// IDI public keys
		antinet_crypto::c_multisign m_IDI_IDC_sig;	/// 'signature' - msg=IDC_pub, signer=IDI

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
//		c_rpc_server m_rpc_server;
//		/**
//		 * @brief rpc_add_limit_points
//		 * @param peer_ip peer hash ip
//		 */
//		bool rpc_add_limit_points(const std::string &peer_ip);
};

// ------------------------------------------------------------------
