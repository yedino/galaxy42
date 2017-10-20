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
#include <json.hpp>
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
		/// Creates tunserver listening on given #port and #rpc_port
		/// @param early_argm is the early state of argm (possibly will be changed later e.g. by devel/demo options - see set_argm)
		/// this #early_argm is only to peek at some options needed early, for example options stored here as const,
		/// like #m_option_insecure_cap
		c_tunserver(int port, int rpc_port, const boost::program_options::variables_map & early_argm);

		/// to easily call constructor without providing real #early_argm, e.g. when using in an unittest
		static boost::program_options::variables_map get_default_early_argm();

		void set_desc(shared_ptr< boost::program_options::options_description > desc);
		void set_argm(shared_ptr< boost::program_options::variables_map > argm);

		void configure_mykey(const std::string &ipv6_prefix); ///<  load my (this node's) keypair
		void run(int time = 0); ///< run the main loop

		/// @name Functions that execute a program action like creation of key, calculating signature, etc.
		/// @{
		void program_action_set_IDI(const string & keyname); ///< set configured IDI key (write the config to disk) @deprecated
		void program_action_gen_key(const boost::program_options::variables_map & argm); ///< generate a key according to given options @deprecated
		std::string program_action_gen_key_simple(); ///< generates recommended simple key, returns name e.g. "IDI" @deprecated
		/// @}

		void set_my_name(const string & name); ///< set a nice name of this peer (shown in debug for example)
		const antinet_crypto::c_multikeys_pub & read_my_IDP_pub() const; ///< read the pubkey of the (main / permanent) ID of this server
		string get_my_ipv6_nice() const; ///< returns the main HIP IPv6 of this node in a nice format (e.g. hexdot)
		int get_my_stats_peers_known_count() const; ///< get the number of currently known peers, for information

		bool add_peer(const t_peering_reference & peer_ref); ///< add this as peer (just from reference), @returns true if peer added or false if peer already exists
		void add_peer_to_black_list(const c_haship_addr & hip); ///< add this to black list
		void add_peer_simplestring(const string & simple); ///< add this as peer, from a simple string like "ip-pub" TODO(r) instead move that to ctor of t_peering_reference
		void add_peer_simplestring_new_format(const string & simple); ///< add this as peer, from a simple string new format
		bool delete_peer(const c_haship_addr &hip); ///< delete this as peer, @return true if peer deleted, false if peer not found
		void delete_peer_from_black_list(const c_haship_addr & hip); ///< delete this from black list
		bool delete_peer_simplestring(const string & simple, bool is_banned); ///< delete this as peer, from a simple string if is_banned==true also add peer to black list
		void delete_all_peers(bool is_banned); ///< delete all peers if is_banned=true also add peer to black list
		///! add this user (or append existing user) with his actuall public key data
		void add_peer_append_pubkey(const t_peering_reference & peer_ref, unique_ptr<c_haship_pubkey> && pubkey);
		void add_tunnel_to_pubkey(const c_haship_pubkey & pubkey);

		typedef enum {
			e_route_method_from_me=1, ///< I am the oryginal sender (try hard to send it)
			e_route_method_if_direct_peer=2, ///< Send data only if if I know the direct peer (e.g. I just route it for someone else - in star protocol the center node)
			e_route_method_default=3, ///< The default routing method
		} t_route_method;

		typedef enum {
			eIPv6_TCP=6,
			eIPv6_UDP=17,
			eIPv6_ICMP=58
		} t_ipv6_protocol_type;

		void nodep2p_foreach_cmd(c_protocol::t_proto_cmd cmd, string_as_bin data) override;
		const c_peering & get_peer_with_hip( c_haship_addr addr , bool require_pubkey ) override;
		int get_my_port() const;
		std::string get_my_reference() const;
		bool check_ip_protocol(const std::string& data) const;
		int get_ip_protocol_number(const std::string& data) const;
		void enable_remove_peers();
		void set_remove_peer_tometout(unsigned int timeout_seconds);
		void set_prefix_len(int prefix);
		void set_prefix(const std::string &prefix);

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
		Mutex & get_my_mutex() const; ///< [thread] get lock guard on this
		friend class c_httpdbg_raport; ///< this is authorized to read my data for debug. but [thread] lock access first!!!
        #endif
		int m_prefix_len;
		std::string m_ipv6_prefix; // i.e. "fd42"
	private:
        #ifdef HTTP_DBG
		mutable Mutex m_my_mutex; ///< [thread] lock this before woring on this class (to protect from access from e.g. httpdbg)
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
                c_event_manager_asio m_event_manager;
		#elif defined(__MACH__)
                c_tun_device_apple m_tun_device;
                c_udp_wrapper_asio m_udp_device;
                c_event_manager_asio m_event_manager;
		#else
		c_tun_device_empty m_tun_device;
		c_udp_wrapper_empty m_udp_device;
		c_event_manager_empty m_event_manager;
		#endif
		unsigned char m_tun_header_offset_ipv6; ///< current offset in TUN/TAP data to the position of ipv6

		shared_ptr< boost::program_options::options_description > m_desc; ///< The boost program options that I will be using. (Needed for some internal commands)
        shared_ptr< boost::program_options::variables_map > m_argm;
//		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		using t_peers_by_haship = std::map< c_haship_addr, unique_ptr<c_peering> >; ///< peers (we always know their IPv6 - we assume here), indexed by their hash-ip
		mutable Mutex m_peer_etc_mutex; ///< one mutex protects m_peer and m_peer_black_list for avoid possible deadlocks
		t_peers_by_haship m_peer GUARDED_BY(m_peer_etc_mutex); ///< my peers, indexed by their hash-ip. MUST BE used only protected by m_peer_etc_mutex!
		std::set<c_haship_addr> m_peer_black_list GUARDED_BY(m_peer_etc_mutex); ///< my peers black list, indexed by their hash-ip. MUST BE used only protected by m_peer_etc_mutex!

		t_peers_by_haship m_nodes; ///< all the nodes that I know about to some degree

		antinet_crypto::c_multikeys_PAIR m_my_IDC; ///< my keys!
		antinet_crypto::c_multikeys_pub	m_my_IDI_pub;	/// IDI public keys
		antinet_crypto::c_multisign m_IDI_IDC_sig;	/// 'signature' - msg=IDC_pub, signer=IDI

		c_haship_addr m_my_hip; ///< my HIP that results from m_my_IDC, already cached in this format

		std::map< c_haship_addr, unique_ptr<c_tunnel_use> > m_tunnel; ///< my crypto tunnels

		bool enable_remove=false; // if false then just count, do not remove
		std::chrono::seconds peer_timeout;

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

		/**
		 * @brief check_packet_destination_address
		 * @param address ipv6 address
		 * @param packet ipv6 packet
		 * @return true if dst address is the same as packet address
		 */
		bool check_packet_destination_address(const std::array<uint8_t, 16> &address_expected, const std::string &packet);

		/**
		 * @brief check_packet_source_address
		 * @param address ipv6 address
		 * @param packet ipv6 packet
		 * @return true if dst address is the same as packet src address
		 */
		bool check_packet_source_address(const std::array<uint8_t, 16> &address_expected, const std::string &packet);

		bool check_packet_address(const std::array<uint8_t, 16> &address_expected, const std::string &packet, const size_t offset);



		c_rpc_server m_rpc_server;
		nlohmann::json rpc_ping(const std::string &input_json);
		nlohmann::json rpc_peer_list(const std::string &input_json);
		nlohmann::json rpc_sending_test(const std::string &input_json);
		nlohmann::json rpc_add_peer(const std::string &input_json);
		nlohmann::json rpc_delete_peer(const std::string &input_json);
		nlohmann::json rpc_delete_all_peers(const std::string &input_json);
		nlohmann::json rpc_ban_peer(const std::string &input_json);
		nlohmann::json rpc_ban_list(const std::string &input_json);
		nlohmann::json rpc_ban_all_peers(const std::string &input_json);
		nlohmann::json rpc_get_galaxy_ipv6(const std::string &input_json);
		nlohmann::json rpc_get_galaxy_invitation(const std::string &input_json);
		nlohmann::json rpc_hello(const std::string &input_json);
		bool peer_on_black_list(const c_haship_addr &hip); ///< @returns true if peer is on black list

		int m_port;
		std::atomic<bool> m_unban_if_banned; ///< if false rpc_add_peer not works for peers on balck list
		std::vector<t_ipv6_protocol_type> m_supported_ip_protocols;

		const bool m_option_insecure_cap; ///< should we do insecure cap (e.g. do NOT drop the capabilities); tests/debug
		t_peering_reference parse_peer_simplestring(const string& simple);
};

// ------------------------------------------------------------------
