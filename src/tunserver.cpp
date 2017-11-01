// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

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


//const char * g_the_disclaimer =
//"*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere! ***"; // XXX

// The name of the hardcoded default demo that will be run with --devel (unless option --develdemo is given) can be set here:
const char * g_demoname_default = "route_dij";
// see function run_mode_developer() here to see list of possible values


#include "libs1.hpp"

#include "tunserver.hpp"

#include "cpputils.hpp"
#include "glue_sodiumpp_crypto.hpp"

#include "ui.hpp"
#include "datastore.hpp"
#include "trivialserialize.hpp"
#include "counter.hpp"
#include "generate_crypto.hpp"



#ifdef __linux__  // for low-level Linux-like systems TUN operations
#include "../depends/cjdns-code/NetPlatform.h" // from cjdns
// linux (and others?) select use:
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include <fcntl.h> // O_RDWRO_RDWR
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ip_icmp.h>	//Provides declarations for icmp header
#include <netinet/udp.h>		//Provides declarations for udp header
#include <netinet/tcp.h>		//Provides declarations for tcp header
#include <netinet/ip.h>			//Provides declarations for ip header
#include <linux/if_tun.h>
#endif

#include "c_json_load.hpp"
#include "c_ip46_addr.hpp"
#include "c_peering.hpp"
#include "generate_crypto.hpp"
#include <json.hpp>

#include "crypto/crypto.hpp" // for tests
#include "rpc/rpc.hpp"


#include "trivialserialize.hpp"
#include "galaxy_debug.hpp"

#include "glue_sodiumpp_crypto.hpp" // e.g. show_nice_nonce()

#include "ui.hpp"

#include "tunserver.hpp"

#include "utils/privileges.hpp"
#include <boost/any.hpp>
#include <galaxysrv_peers.hpp>

//const char * g_the_disclaimer =
//"*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere! ***"; // XXX
//const char * g_the_disclaimer = gettext("L_warning_work_in_progress");


// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	_throw_error( std::runtime_error(msg) );
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

const char* expected_not_found_missing_pubkey::what() const noexcept {
		return "expected_not_found_missing_pubkey";
}


// ------------------------------------------------------------------

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
		const auto peer_pub_ptr = peer.get_pub();
		if (!peer_pub_ptr) _throw_error( std::runtime_error("This peer has no pubkey yet.") );
		c_route_info route_info( peer.get_hip() , cost , * peer_pub_ptr );
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
			_throw_error( std::runtime_error("no route known (and we do NOT WANT TO search) to dst=" + STR(dst)) );
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
	_throw_error( std::runtime_error("NO ROUTE known (at current time) to dst=" + STR(dst)) );
}

void  c_routing_manager::c_route_search::execute( c_galaxy_node & galaxy_node ) {
	_info("Sending QUERY for HIP, with m_ttl_should_use=" << m_ttl_should_use);
	string_as_bin data; // [protocol] for search query - format is: HIP_BINARY;TTL_BINARY;

	data += string_as_bin(m_addr);
	data += string(";");

	unsigned char byte_highest_ttl = m_ttl_should_use;  assert( m_ttl_should_use == byte_highest_ttl ); // TODO(r) asserted narrowing
	data += string(1, static_cast<char>(byte_highest_ttl) );
	data += string(";");

	galaxy_node.nodep2p_foreach_cmd( c_protocol::t_proto_cmd::e_proto_cmd_findhip_query , data );

	m_ttl_used = byte_highest_ttl;
	m_ask_time = std::chrono::steady_clock::now();
}

// ------------------------------------------------------------------

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


using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

t_peering_reference c_tunserver::parse_peer_simplestring(const string& simple)
{
	size_t pos1 = simple.find('-');
	if (pos1 == std::string::npos) throw std::out_of_range("");
	string part_pip = simple.substr(0,pos1);
	string part_hip = simple.substr(pos1+1);
	_info("Peer pip="<<part_pip<<" hip="<<part_hip);
	auto ip_pair = tunserver_utils::parse_ip_string(part_pip);
	_note("Physical IP: address=" << ip_pair.first << " port=" << ip_pair.second);
	return t_peering_reference( ip_pair.first, ip_pair.second , part_hip );
}

void c_tunserver::add_peer_simplestring(const string & simple) {
	// TODO delete_newloop
	_dbg1("Adding peer from simplestring=" << simple);
	// "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b"
	try {
		t_peering_reference peering_ref = parse_peer_simplestring(simple);
		add_peer(peering_ref);
		delete_peer_from_black_list(peering_ref.haship_addr);
	}
	catch (const std::exception &e) {
//		_erro("Adding peer from simplereference failed (exception): " << e.what());
                _erro(mo_file_reader::gettext("L_failed_adding_peer_simple_reference") << e.what());

//                _throw_error( std::invalid_argument("Bad peer format") );
		_throw_error( std::invalid_argument(mo_file_reader::gettext("L_bad_peer_format")) );
	}
}

void c_tunserver::add_peer_simplestring_new_format(const string &simple)
{
	try {
		c_galaxysrv_peers::t_peering_reference_parse parse = c_galaxysrv_peers::parse_peer_reference(simple); // partially parsed
		_check_input(parse.first.size() > 0);
		_check_input(parse.second.size() > 0);
		std::string cable = parse.second[0];
		// udp:192.166.218.58:19042
		auto begin = cable.find(':');
		_check_input(begin != std::string::npos);
		std::string ipv4_with_port = cable.substr(begin+1);
		auto end = ipv4_with_port.find(':');
		_check_input(end != std::string::npos);
		std::string ipv4 = ipv4_with_port.substr(0, end);
		t_peering_reference peering_ref(ipv4, parse.first[0]);
		add_peer(peering_ref);
		delete_peer_from_black_list(peering_ref.haship_addr);
	}
	catch (const std::exception &) {
		throw err_check_input("Bad peer new format");
	}
}

bool c_tunserver::delete_peer(const c_haship_addr &hip)
{
	_mark("delete_peer (delete only!) " << hip);
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		auto iter = m_peer.find(hip);
		if (iter != m_peer.end()) {
			m_peer.erase(iter);
			return true;
		}
		return false;
	}
}

void c_tunserver::delete_peer_from_black_list(const c_haship_addr &hip)
{
	_mark("delete peer from black list (delete only!) " << hip);
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		auto iter = m_peer_black_list.find(hip);
		if (iter != m_peer_black_list.end())
			m_peer_black_list.erase(iter);
	}
}

bool c_tunserver::delete_peer_simplestring(const string &simple, bool is_banned)
{
	_dbg1("Deleting peer from simplestring=" << simple);
	try {
		c_haship_addr hip(c_haship_addr::tag_constr_by_addr_dot(), simple);
		bool peer_deleted = delete_peer(hip);
		if (is_banned)
			add_peer_to_black_list(hip);
		return peer_deleted;
	}
	catch (const std::exception &e) {
		_erro(mo_file_reader::gettext("L_failed_deleting_peer_simple_reference") << e.what());
		_throw_error( std::invalid_argument(mo_file_reader::gettext("L_bad_peer_format")) );
	}
}

void c_tunserver::delete_all_peers(bool is_banned)
{
	_mark("delete all peers (delete only!) ");
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		if (is_banned)
		{
			for( auto &peer : m_peer)
				m_peer_black_list.emplace(peer.first);
		}
		m_peer.clear();
	}
}

c_tunserver::c_tunserver(int port, int rpc_port, const boost::program_options::variables_map & early_argm)
:
	m_my_name("unnamed-tunserver")
	,m_udp_device(port)
	,m_event_manager(m_tun_device, m_udp_device)
	,m_tun_header_offset_ipv6(0)
	,m_rpc_server(rpc_port)
	,m_port(port)
	,m_unban_if_banned(true)
	,m_supported_ip_protocols{eIPv6_TCP, eIPv6_UDP, eIPv6_ICMP}
	,m_option_insecure_cap( early_argm.at("insecure-cap").as<bool>() )
{
	if (m_option_insecure_cap) _warn("INSECURE OPTION is active: m_option_insecure_cap");
	m_rpc_server.add_rpc_function("ping", [this](const std::string &input_json) {
		return rpc_ping(input_json);
	});
	m_rpc_server.add_rpc_function("peer_list", [this](const std::string &input_json) {
		return rpc_peer_list(input_json);
	});
	m_rpc_server.add_rpc_function("sending_test", [this](const std::string &input_json) {
		return rpc_sending_test(input_json);
	});
	m_rpc_server.add_rpc_function("add_peer", [this](const std::string &input_json) {
		return rpc_add_peer(input_json);
	});
	m_rpc_server.add_rpc_function("delete_peer", [this](const std::string &input_json) {
		return rpc_delete_peer(input_json);
	});
	m_rpc_server.add_rpc_function("delete_all_peers", [this](const std::string &input_json) {
		return rpc_delete_all_peers(input_json);
	});
	m_rpc_server.add_rpc_function("ban_peer", [this](const std::string &input_json) {
		return rpc_ban_peer(input_json);
	});
	m_rpc_server.add_rpc_function("ban_all_peers", [this](const std::string &input_json) {
		return rpc_ban_all_peers(input_json);
	});
	m_rpc_server.add_rpc_function("get_galaxy_ipv6", [this](const std::string &input_json) {
		return rpc_get_galaxy_ipv6(input_json);
	});
	m_rpc_server.add_rpc_function("get_galaxy_new_format_reference", [this](const std::string &input_json) {
		return rpc_get_galaxy_invitation(input_json);
	});
	m_rpc_server.add_rpc_function("hello", [this](const std::string &input_json) {
		return rpc_hello(input_json);
	});
}

boost::program_options::variables_map c_tunserver::get_default_early_argm() {
	boost::program_options::variables_map early_argm;
	boost::any x(false);
	early_argm.insert( std::make_pair("insecure-cap"  ,  boost::program_options::variable_value(false,false) ) );
	return early_argm;
}

#ifdef HTTP_DBG
Mutex & c_tunserver::get_my_mutex() const {
	return this->m_my_mutex; // TODO or const-cast here? from mutable?
}
#endif

void c_tunserver::set_desc(shared_ptr< boost::program_options::options_description > desc) {
	m_desc = desc;
}

void c_tunserver::set_argm(shared_ptr< boost::program_options::variables_map > argm) {
	_check(argm);
	m_argm = argm;
  _check( m_option_insecure_cap == UsePtr(m_argm).at("insecure-cap").as<bool>() ); // should not change (e.g. vs ctor early_argm)
}

void c_tunserver::set_my_name(const string & name) {  m_my_name = name; _note("This node is now named: " << m_my_name);  }

const antinet_crypto::c_multikeys_pub & c_tunserver::read_my_IDP_pub() const {
	return m_my_IDI_pub;
}

string c_tunserver::get_my_ipv6_nice() const {
	return m_my_IDI_pub.get_ipv6_string_hexdot();
}


int c_tunserver::get_my_stats_peers_known_count() const {
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	return m_peer.size();
}

// my key @deprecated (newloop)
void c_tunserver::configure_mykey(const std::string &ipv6_prefix) {
	// creating new IDC from existing IDI // this should be separated
	//and should include all chain IDP->IDM->IDI etc.  sign and verification

	// getting IDC
	std::string IDI_name;
	try {
		IDI_name = datastore::load_string(e_datastore_galaxy_instalation_key_conf, "IDI");
	} catch (std::invalid_argument &err) {
		_dbg2("IDI is not set");
		_throw_error( std::runtime_error("IDI is not set") );
	}

	std::unique_ptr<antinet_crypto::c_multikeys_PAIR> my_IDI;
	my_IDI = std::make_unique<antinet_crypto::c_multikeys_PAIR>();
	//my_IDI->set_ipv6_prefix(ipv6_prefix);
	my_IDI->set_ipv6_prefix(m_ipv6_prefix);
	my_IDI->datastore_load_PRV_and_pub(IDI_name);
	// getting HIP from IDI
	auto IDI_ip_bin = my_IDI->get_ipv6_string_bin() ;
	auto IDI_ip_hexdot = my_IDI->get_ipv6_string_hexdot() ;
	// IDI_hexdot.at(0)='z'; // for testing
	try {
		std::ostringstream oss; oss<<"Your Hash-IP address looks not valid (not a Galaxy42 address?)"
			<< " - the IDI part is: ["<<IDI_ip_hexdot<<"]";
		const string msg_badip = oss.str();
		if (static_cast<unsigned char>(IDI_ip_bin.at(0)) != 0xFD) throw std::runtime_error(msg_badip);
		if (IDI_ip_hexdot.at(0) != 'f') throw std::runtime_error(msg_badip);
		if (IDI_ip_hexdot.at(1) != 'd') throw std::runtime_error(msg_badip);
	}
	catch UI_CATCH_RETHROW("Testing your Hash-IP (IDI)");

	c_haship_addr IDI_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDI_ip_hexdot );
	_info("IDI IPv6: " << IDI_ip_hexdot);
	_dbg1("IDI IPv6: " << IDI_hip << " (other var type)");
	// creating IDC for this session
	antinet_crypto::c_multikeys_PAIR my_IDC;
	my_IDC.generate(antinet_crypto::e_crypto_system_type_X25519,1);
	// signing it by IDI
	std::string IDC_pub_to_sign = my_IDC.m_pub.serialize_bin();
	antinet_crypto::c_multisign IDC_IDI_signature = my_IDI->multi_sign(IDC_pub_to_sign);

	// example veryifying
	antinet_crypto::c_multikeys_pub::multi_sign_verify(IDC_IDI_signature, IDC_pub_to_sign, my_IDI->m_pub);

	// save signature and IDI publickey in tunserver
	m_my_IDI_pub = my_IDI->m_pub;
	m_my_IDI_pub.set_ipv6_prefix(m_ipv6_prefix);
	m_IDI_IDC_sig = IDC_IDI_signature;

	// remove IDP from RAM
	// (use of locked_string should take care of actually shreding memory)
	my_IDI.reset(nullptr);

	// for debug, hip from IDC
	auto IDC_ip_hexdot = my_IDC.get_ipv6_string_hexdot() ;

	try {
		std::ostringstream oss; oss<<"Your Hash-IP address looks not valid (not a Galaxy42 address?)"
			<< " - the IDC part is: ["<<IDC_ip_hexdot<<"]";
		const string msg_badip = oss.str();
		if (IDC_ip_hexdot.at(0) != 'f') throw std::runtime_error(msg_badip);
		if (IDC_ip_hexdot.at(1) != 'd') throw std::runtime_error(msg_badip);
	}
	catch UI_CATCH_RETHROW("Testing your Hash-IP (IDC)");

	c_haship_addr IDC_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDC_ip_hexdot );
	_info("IDC IPv6: " << IDC_ip_hexdot);
	_dbg1("IDC IPv6: " << IDC_hip << " (other var type)");
	// now we can use hash ip from IDI and IDC for encryption
	m_my_hip = IDI_hip;
	m_my_IDC = my_IDC;
	m_my_IDC.set_ipv6_prefix(m_ipv6_prefix);
}

// add peer
bool c_tunserver::add_peer(const t_peering_reference & peer_ref) { ///< add this as peer
	_mark("add_peer (add only!) " << peer_ref );
	auto peering_ptr = make_unique<c_peering_udp>(peer_ref, m_udp_device);
	// key is unique in map
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		auto ret_pair = m_peer.emplace( std::make_pair( peer_ref.haship_addr ,  std::move(peering_ptr) ) );
		return ret_pair.second;
	}
}

void c_tunserver::add_peer_to_black_list(const c_haship_addr &hip)
{
	_mark("add_peer to black list (add only!) " << hip );
	// key is unique in map
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		m_peer_black_list.emplace(std::move(hip));
	}
}

void c_tunserver::add_peer_append_pubkey(const t_peering_reference & peer_ref,
unique_ptr<c_haship_pubkey> && pubkey)
{
	auto peer_hip =  peer_ref.haship_addr;
	_dbg1("Update (or add) peer reference for hip="<<peer_hip<< " , reference=" << peer_ref << ", pubkey=" << (*pubkey));
	{
		// auto hip_from_pubkey = pubkey->get_ipv6_string_hexdot(); // re-confirm there that the haship matches the pubkey, just to be sure:
		c_haship_addr hip_from_pubkey( c_haship_addr::tag_constr_by_hash_of_pubkey() , *pubkey );

		if (hip_from_pubkey != peer_ref.haship_addr) {
			_throw_error(runtime_error(join_string_sep("Pubkey",*pubkey," has hip not matching reference",peer_ref)));
		}
	}

	// Adding new peer peering{ peering-addr=192.168.1.108:9042 hip=hip:fd42e5ca4e2acd1354355e4e45bfe4da pub=(null)} with pubkey=pub:41:[G,M,K,a,o,0x1,e,0x1 ... w,0xF0=240,),0x1F=31]

	{ // lock
		LockGuard<Mutex> lg(m_peer_etc_mutex);

		try {
			_dbg2("We have him ALREADY in map: " << to_debug( m_peer.at( peer_hip ) ) );
		} catch(...) { _warn("This peer is not yet in map"); }

		auto find = m_peer.find( peer_hip );
		if (find == m_peer.end()) { // no such peer yet
			auto iter = m_peer_black_list.find( peer_hip );
			if (iter == m_peer_black_list.end()) { // no such peer on black list yet
				auto peering_ptr = make_unique<c_peering_udp>(peer_ref, m_udp_device);
				_fact("Adding NEW peer " << peer_ref << " with pubkey=" << (*pubkey));
				peering_ptr->set_pubkey(std::move(pubkey));
				_fact("Adding NEW peer reference: " << to_debug(peering_ptr));
				m_peer.emplace( std::make_pair( peer_hip ,  std::move(peering_ptr) ) );
			}
		} else { // update existing
			auto & peering_ptr = find->second;
			peering_ptr->update_last_ping_time();
			const auto & old_pip = peering_ptr->get_pip();
			const auto & new_pip = peer_ref.peering_addr;
			peering_ptr->set_pubkey(std::move(pubkey)); // append the pubkey!
			if (old_pip == new_pip) {
				_info("This peer indexed by hip="<<peer_hip<<" has unchanged IP "<< new_pip);
			}
			else {
				_fact("This peer indexed by hip="<<peer_hip<<" CHANGES IP (physical), from " << old_pip << " to " << new_pip);
				peering_ptr->set_pip( new_pip );
			}
		}

		try {
			_dbg2("As result, after adding/appending peer, we have him in map: " << to_debug( m_peer.at( peer_hip ) ) );
		} catch(...) { _erro("Failed to display the added peer"); throw ; }

	} // lock

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


//#ifdef __linux__
void c_tunserver::prepare_socket() {
	ui::action_info_ok("Allocated virtual network card interface (TUN)"); // TODO@mik translate './menu lc'
	// under name: " + to_string(ifr.ifr_name));

	m_tun_header_offset_ipv6 = g_tuntap::header_position_of_ipv6; // matching the TUN/TAP type above
	{
		std::array<uint8_t, 16> address;
		assert(m_my_hip.size() == 16 && "m_my_hip != 16");
		for (int i=0; i<16; ++i) address[i] = m_my_hip[i];
		// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
		// ...to the device to which we are setting IP address here:
		assert(address[0] == 0xFD);
//		assert(address[1] == 0x42);

		_fact("Will configure the tun device");
		try {
			m_tun_device.init();
			m_tun_device.set_ipv6_address(address, m_prefix_len);
			m_tun_device.set_mtu(1304);

			_fact("Done init of event manager - for this tuntap");
			m_event_manager.init(); // because now we have the tuntap fully ready (with the fd)
		}
		catch (tuntap_error_devtun &ex) { ui::action_error_exit("Problem with setup of virtual card (tun/tap) with accessing tun/tap driver-file; "s + ex.what()); }
		catch (tuntap_error_ip &ex) { ui::action_error_exit("General problem with setup of virtual card (tun/tap) while setting up virtual IP address; "s + ex.what()); }
		catch (tuntap_error_mtu &ex) { ui::action_error_exit("General problem with setup of virtual card (tun/tap) while configuring the MTU option; "s + ex.what()); }
		catch (tuntap_error &ex) { ui::action_error_exit("General problem with setup of virtual card (tun/tap); "s + ex.what()); }
		catch (std::exception &ex) { ui::action_error_exit("Unknon problem (std::exception) with setup of virtual card (tun/tap) "s + ex.what()); }
		catch (...) { ui::action_error_exit("Unknon problem (unknown exception type) with setup of virtual card (tun/tap)."); }
		_goal("Tun device seems fully configured");
	}

	#ifdef __linux__
		_assert(m_udp_device.get_socket() >= 0);
	#endif

	ui::action_info_ok("Started virtual network card interface (TUN)"
	//under name: " + to_string(ifr.ifr_name)
		+ std::string(" with proper IPv6 and other settings"));
}


#ifdef __linux__
void c_tunserver::wait_for_fd_event() { // wait for fd event
	_info("Selecting");
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_udp_device.get_socket(), &m_fd_set_data);

	auto fd_max = m_udp_device.get_socket(); // = std::max(m_tun_fd, m_udp_device.get_socket());
	_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	_assert(fd_max >= 1);

	timeval timeout { 0 , 500 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html

	auto select_result = select( fd_max+1, &m_fd_set_data, NULL, NULL, & timeout); // <--- blocks
	_assert(select_result >= 0);
}
#else
	//void c_tunserver::prepare_socket(){}
#endif

std::pair<c_haship_addr,c_haship_addr> c_tunserver::parse_tun_ip_src_dst(const char *buff, size_t buff_size) { ///< the same, but with ipv6_offset that matches our current TUN
	return parse_tun_ip_src_dst(buff,buff_size, m_tun_header_offset_ipv6 );
}

std::pair<c_haship_addr,c_haship_addr> c_tunserver::parse_tun_ip_src_dst(const char *buff, size_t buff_size, unsigned char ipv6_offset) {
	size_t pos_src = ipv6_offset + g_ipv6_rfc::header_position_of_src , len_src = g_ipv6_rfc::header_length_of_src;
	size_t pos_dst = ipv6_offset + g_ipv6_rfc::header_position_of_dst , len_dst = g_ipv6_rfc::header_length_of_dst;
	if(buff_size < pos_src+len_src) throw std::runtime_error("undersized buffer");
	if(buff_size < pos_dst+len_dst) throw std::runtime_error("undersized buffer");
	// valid: reading pos_src up to +len_src, and same for dst

#ifdef __linux__

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

// __linux__
#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MACH__)
	using namespace boost::asio;
	ip::address_v6::bytes_type ip_bytes;
	std::copy_n(buff + pos_src, ip_bytes.size(), ip_bytes.begin());
	ip::address_v6 ip6_addr(ip_bytes);
	_dbg1("src ipv6_str " << ip6_addr);
	c_haship_addr ret_src(c_haship_addr::tag_constr_by_addr_dot(), ip6_addr.to_string());

	std::copy_n(buff + pos_dst, ip_bytes.size(), ip_bytes.begin());
	ip6_addr = ip::address_v6(ip_bytes);
	_dbg1("dst ipv6_str " << ip6_addr);
	c_haship_addr ret_dst(c_haship_addr::tag_constr_by_addr_dot(), ip6_addr.to_string());

// __win32 || __cygwin__ || __mach__ (multiplatform boost::asio)
#endif

	return std::make_pair( ret_src , ret_dst );
}

void c_tunserver::peering_ping_all_peers() {
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	auto now = std::chrono::steady_clock::now();
	_dbg2("Remove inactive peers, time="<<now);
	size_t count_removed=0; // how many we removed
	if (enable_remove) {
		for (auto it = m_peer.begin(), it_end = m_peer.end(); it != it_end;) {
			auto last_ping_seconds = std::chrono::duration_cast<std::chrono::seconds>(now - it->second->get_last_ping_time()); //< seconds after the last
			if (last_ping_seconds > peer_timeout) {
				_note("removing peer " << it->first.get_hip_as_string(true));
				it = m_peer.erase(it);
				it_end = m_peer.end();
				_dbg1("after erase");
				++count_removed;
			} else {
				++ it;
			}
		}
	}
	if (count_removed) {
		_mark( (enable_remove ? "Removed actually" : "Would remove (but disabled)")
		<< count_removed << " inactive peer(s), time="<<now);
	}

	_info("Sending ping to all peers (count=" << m_peer.size() << ")");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived

		// [protocol] build raw
		trivialserialize::generator gen(8000);
		gen.push_varstring( m_my_IDC.get_serialize_bin_pubkey() );
		gen.push_varstring( m_my_IDI_pub.serialize_bin());
		gen.push_varstring( m_IDI_IDC_sig.serialize_bin());
		string_as_bin cmd_data( gen.str_move() );
		// TODONOW
		peer_udp->send_data_udp_cmd(c_protocol::t_proto_cmd::e_proto_cmd_public_hi, cmd_data, m_udp_device.get_socket());
	}
}

void c_tunserver::nodep2p_foreach_cmd(c_protocol::t_proto_cmd cmd, string_as_bin data) {
	_info("Sending a COMMAND to peers:");
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived
		peer_udp->send_data_udp_cmd(cmd, data, m_udp_device.get_socket());
	}
}

const c_peering & c_tunserver::get_peer_with_hip( c_haship_addr addr , bool require_pubkey ) {
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	auto peer_iter = m_peer.find(addr);
	if (peer_iter == m_peer.end()) {
		_dbg4("this HIP is not in peers");
		throw expected_not_found();
	}
	c_peering & peer = * peer_iter->second;
	if (require_pubkey) {
		if (! peer.is_pubkey()) _throw_error( expected_not_found_missing_pubkey() );
	}
	return peer;
}

void c_tunserver::debug_peers() {
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	if (!m_peer.size()) _fact("You have no peers currently.");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		_fact("  * Known peer on key [ " << v.first << " ] => " << (* target_peer) );
	}
}

bool c_tunserver::route_tun_data_to_its_destination_detail(t_route_method method,
	const char *buff, size_t buff_size,
	c_haship_addr src_hip, c_haship_addr dst_hip,
	c_haship_addr next_hip,
	c_routing_manager::c_route_reason reason,
	int recurse_level, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used)
{
	if (data_route_ttl<=0) { _warn("TTL expended. NOT routing.");	return false;	}
	UniqueLockGuardRW<Mutex> lg(m_peer_etc_mutex);
	if (m_peer.size() == 0) {
		_warn("I have no peers, I can not route anywhere.");
		return false;
	}

	// --- choose next hop in peering ---

	// try direct peers:

	// find c_peering to send to // TODO(r) this functionallity will be soon
	// doubled with the route search in m_routing_manager below, remove it then
	auto peer_it = m_peer.find(next_hip);

	if (peer_it == m_peer.end()) { // not a direct peer!
		lg.unlock();
		_info("ROUTE: can not find in direct peers next_hip="<<next_hip);
		if (recurse_level>1) {
			_warn("DROP: Recursion level too big in choosing peer");
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
		lg.unlock();
		// send it on wire:
		peer_udp->send_data_udp(buff, buff_size, m_udp_device.get_socket(), src_hip, dst_hip, data_route_ttl, nonce_used); // <--- *** actually send the data
	}
	return true;
}

bool c_tunserver::route_tun_data_to_its_destination_top(t_route_method method,
	const char *buff, size_t buff_size,
	c_haship_addr src_hip, c_haship_addr dst_hip,
	c_routing_manager::c_route_reason reason, int data_route_ttl, antinet_crypto::t_crypto_nonce nonce_used)
{
	if (data_route_ttl<=0) { _warn("TTL expended. NOT routing.");	return false;	}
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
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	for(auto & v : m_peer) { if ((v.second->get_pip() == ip) && (v.second->get_pip().get_assigned_port() == ip.get_assigned_port())) return * v.second.get(); }
	_throw_error( std::runtime_error("We do not know a peer with such IP=" + STR(ip)) );
}

bool c_tunserver::check_packet_destination_address(const std::array<uint8_t, 16> &address_expected, const string &packet) {
	return check_packet_address(address_expected, packet, g_ipv6_rfc::header_position_of_dst); //< 28 == offset of src address
}

bool c_tunserver::check_packet_source_address(const std::array<uint8_t, 16> &address_expected, const string &packet) {
	return check_packet_address(address_expected, packet, g_ipv6_rfc::header_position_of_src); //< 12 == offset of src address
}

bool c_tunserver::check_packet_address(const std::array<uint8_t, 16> &address_expected, const string &packet, const size_t offset) {
	if (packet.size() < 40) // 40 = ipv6 header size
		throw std::invalid_argument("Packet is too small (smaller than ipv6 header)");
	if (offset > packet.size() - 16)
		throw std::invalid_argument("too big offset");
	std::string packet_address(packet, offset, 16); // from substring
	assert(address_expected.size() == packet_address.size());
	assert(packet_address.size() == 16); // ipv6 address size == 16
	for (int i = 0; i < 16; i++) {
		if (address_expected.at(i) != static_cast<uint8_t>(packet_address.at(i))) {
			return false;
		}
	}
	return true;
}

nlohmann::json c_tunserver::rpc_ping(const string &input_json) {
	_UNUSED(input_json); // TODO?
	//Json::Value input(input_json);
	nlohmann::json ret;
	ret["cmd"] = "ping";
	ret["msg"] = "pong";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_peer_list(const string &input_json) {
	_UNUSED(input_json);
	nlohmann::json ret;
	std::vector<std::string> refs;
	// ipv4:port-ipv6
	UniqueLockGuardRW<Mutex> lg(m_peer_etc_mutex);
	for (const auto &peer : m_peer) {
		std::ostringstream oss;
		oss << peer.second->get_pip();
		oss << "-";
		auto hip = peer.second->get_hip();
		for (size_t i = 0; i < hip.size(); i+=2) {
			uint16_t block = 0;
			block = hip[i] << 8;
			block += hip[i+1];
			oss << std::hex << block << ":";
		}
		oss << std::dec;
		refs.emplace_back(oss.str());
		refs.back().erase(refs.back().end() - 1); // remove last character (':')
	}
	lg.unlock();
	ret["cmd"] = "peer_list";
	ret["peers"] = refs;
	ret["msg"] = "ok:";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_sending_test(const string &input_json) {
	auto input = nlohmann::json::parse(input_json);
	auto packet_size = input["size"].get<size_t>();
	auto packet_count = input["count"].get<size_t>();
	c_peering_udp my_peer(t_peering_reference("127.0.0.1", m_port, m_my_hip.get_hip_as_string(true)), m_udp_device);
	std::vector<char> packet(packet_size);
	antinet_crypto::t_crypto_nonce nonce;
	const auto time_start = std::chrono::steady_clock::now();
	for (size_t i = 0; i < packet_count; i++) {
		my_peer.send_data_udp(packet.data(), packet.size(), m_udp_device.get_socket(), m_my_hip, m_my_hip, 3, nonce);
	}
	const auto time_stop = std::chrono::steady_clock::now();
	auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_stop - time_start).count();
	nlohmann::json ret;
	ret["cmd"] = "sending_test";
	ret["time_ms"] = time_ms;
	ret["speed_mbps"] = ((packet_size * packet_count) / (time_ms / 1000.)) / (1024. * 1024.);
	ret["msg"] = "ok:";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_add_peer(const string &input_json) {
	auto input = nlohmann::json::parse(input_json);
	auto peer = input["peer"].get<std::string>();
	auto format = input["format"].get<std::string>();
	nlohmann::json ret;
	ret["cmd"] = "add_peer";
	auto can_i_add_peer = [this](const c_haship_addr &hip) {
		if (m_unban_if_banned) return true; // I can always add peer
		bool is_peer_on_black_list = peer_on_black_list(hip);
		if (is_peer_on_black_list) return false;
		else return false;
	}; // lambda

	c_haship_addr hip;
	try {
		if (format == "0.1") {
			hip = parse_peer_simplestring(peer).haship_addr;
		}
		else if (format == "1.0") {
			size_t at_position = peer.find('@');
			if (at_position == std::string::npos) throw std::invalid_argument("@ character not found");
			auto hip_str = peer;
			hip_str.erase(at_position);
			hip = c_haship_addr(c_haship_addr::tag_constr_by_addr_dot(), hip_str);
		}
		else throw std::invalid_argument("Bad format");

		if (can_i_add_peer(hip)) { // peer on black list
			if (format == "0.1") add_peer_simplestring(peer);
			else if (format == "1.0") add_peer_simplestring_new_format(peer);
			ret["msg"] = "ok: Peer added";
			ret["state"] = "ok";
		} else {
			ret["msg"] = "failed: peer is banned and you said to not unban him";
			ret["state"] = "blocked";
		}

	} catch(const std::exception &) {
		ret["msg"] = "fail: Bad peer format";
		ret["state"] = "error";
	}
	return ret;
}

nlohmann::json c_tunserver::rpc_delete_peer(const string &input_json)
{
	auto input = nlohmann::json::parse(input_json);
	auto peer = input["peer"].get<std::string>();
	nlohmann::json ret;
	ret["cmd"] = "delete_peer";
	try{
		bool peer_deleted = delete_peer_simplestring(peer, false);
		if (peer_deleted) {
			ret["msg"] = "ok: Peer deleted";
			ret["state"] = "ok";
		} else {
			ret["msg"] = "ok: Peer not found";
			ret["state"] = "nothing_done";
		}
	} catch(const std::invalid_argument &) {
		ret["msg"] = "fail: Bad peer format";
		ret["state"] = "error";
	}
	return ret;
}

nlohmann::json c_tunserver::rpc_delete_all_peers(const string &input_json)
{
	_UNUSED(input_json);
	nlohmann::json ret;
	ret["cmd"] = "delete_all_peers";
	delete_all_peers(false);
	ret["msg"] = "ok: All peers deleted";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_ban_peer(const string &input_json)
{
	auto input = nlohmann::json::parse(input_json);
	auto peer = input["peer"].get<std::string>();
	nlohmann::json ret;
	ret["cmd"] = "ban_peer";
	c_haship_addr hip(c_haship_addr::tag_constr_by_addr_dot(), peer);
	try {
		if (peer_on_black_list(hip)) {
			ret["msg"] = "nothing done, peer is banned";
			ret["state"] = "nothing_done";
		} else {
			delete_peer_simplestring(peer, true);
			ret["msg"] = "ok: Peer banned";
			ret["state"] = "ok";
		}
	} catch(const std::invalid_argument &) {
		ret["msg"] = "fail: Bad peer format";
		ret["state"] = "error";
	}
	return ret;
}

nlohmann::json c_tunserver::rpc_ban_list(const string &input_json) {
	_UNUSED(input_json);
	nlohmann::json ret;
	UniqueLockGuardRW<Mutex> lg(m_peer_etc_mutex);
	ret["peers"] = m_peer_black_list;
	lg.unlock();
	ret["cmd"] = "ban_list";
	ret["msg"] = "ok";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_ban_all_peers(const string &input_json)
{
	_UNUSED(input_json);
	nlohmann::json ret;
	ret["cmd"] = "ban_all_peers";
	delete_all_peers(true);
	ret["msg"] = "ok: All peers banned";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_get_galaxy_ipv6(const string &input_json)
{
	_UNUSED(input_json);
	nlohmann::json ret;
	ret["cmd"] = "get_galaxy_ipv6";
	ret["ipv6"] = get_my_ipv6_nice();
	ret["msg"] = "ok:";
	ret["state"] = "ok";
	return ret;
}

nlohmann::json c_tunserver::rpc_get_galaxy_invitation(const string &input_json)
{
	auto input = nlohmann::json::parse(input_json);
	auto ipv4_list = input["msg"].get<std::vector<std::string> >();
	nlohmann::json ret;
	ret["cmd"] = "get_galaxy_invitation";
	ostringstream oss;
	oss << get_my_ipv6_nice();
	for (auto ipv4 : ipv4_list) {
		oss << "@(udp:" << ipv4 << ':' << get_my_port() << ')';
	}
	ret["inv"] = oss.str();
	ret["msg"] = "ok:";
	return ret;
}

nlohmann::json c_tunserver::rpc_hello(const string &input_json)
{
	_UNUSED(input_json);
	nlohmann::json ret;
	ret["cmd"] = "hello";
	ret["state"] = "ok";
	return ret;
}

bool c_tunserver::peer_on_black_list(const c_haship_addr &hip) {
	LockGuard<Mutex> lg(m_peer_etc_mutex);
	auto it = m_peer_black_list.find(hip); // check if peer is on balck list
	if (it != m_peer_black_list.end()) // peer on black list
		return true;
	return false;
}

void c_tunserver::event_loop(int time) {
//	const char * g_the_disclaimer = gettext("L_warning_work_in_progress");

	_info("Entering the event loop");
	c_counter counter(2,true);
	c_counter counter_big(10,false);

	this->peering_ping_all_peers();

    const auto ping_all_frequency = std::chrono::seconds( m_argm->at("net-hello-interval").as<int>() ); // how often to ping them
	const auto ping_all_frequency_low = std::chrono::seconds( 1 ); // how often to ping first few times
	const long int ping_all_count_low = 2; // how many times send ping fast at first

	auto ping_all_time_last = std::chrono::steady_clock::now(); // last time we sent ping to all
	long int ping_all_count = 0; // how many times did we do that in fact
	const auto idle_banner_frequency = std::chrono::seconds( 15 ); // how often to show banner/stats


	// low level receive buffer
	constexpr size_t buf_size=65536;
	char buf[buf_size];


	bool was_connected=true;
	{
		LockGuard<Mutex> lg(m_peer_etc_mutex);
		if (! m_peer.size()) {
			was_connected=false;
			ui::action_info_ok(mo_file_reader::gettext("L_wait_for_connect"));
		}
	}
	bool was_anything_sent_from_TUN=false, was_anything_sent_to_TUN=false;

        double start = std::clock();
        auto timer = [start](int time){ return ((std::clock() - start) / static_cast<double>(CLOCKS_PER_SEC)) * 1000 < time; };

	auto time_loop_start = std::chrono::steady_clock::now(); // time now
	unique_ptr<decltype(time_loop_start)> time_last_idle = nullptr; // when we last time displayed idle notification; TODO std::optional

	my_cap::verify_privileges_are_as_for_mainloop(); // confirm we are secured for the main loop

	while (time ? timer(time) : true) {
		bool anything_happened{false}; // in given loop iteration, for e.g. debug

		try { // ---

		 // std::this_thread::sleep_for( std::chrono::milliseconds(100) ); // was needeed to avoid any self-DoS in case of TTL bugs
		 // std::this_thread::sleep_for( std::chrono::milliseconds(100) ); // was needeed to avoid any self-DoS in case of TTL bugs

		if (!was_connected) {
			UniqueLockGuardRW<Mutex> lg(m_peer_etc_mutex);
			if (m_peer.size()) { // event: conntected now
				lg.unlock();
				was_connected=true;
				ui::action_info_ok("Ok, we have a peer.");
			}
		}

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
		oss <<	"Node " << m_my_name << " "
			<< " has peer(s): " << get_my_stats_peers_known_count()
			<< " has address: " << m_my_hip ;
		const string node_title_bar = oss.str();

		/*if (anything_happened || 1) {
			debug_peers();
			string xx(10,'-');
			_info('\n' << xx << node_title_bar << xx << "\n\n");
		} // --- print your name ---
		*/

		m_event_manager.wait_for_event();

		// TODO(r): program can be hanged/DoS with bad routing, no TTL field yet
		// ^--- or not fully checked. need scoring system anyway

		if (m_event_manager.get_tun_packet()) { // get packet from tun
			anything_happened=true;
			std::vector<int8_t> tun_read_buff(buf_size);
			auto size_read = m_tun_device.read_from_tun(&tun_read_buff[0], tun_read_buff.size());
			_info("TTTTTTTTTTTTTTTTTTTTTTTTTT ###### ------> TUN read " << size_read << " bytes: [" << string(reinterpret_cast<char *>(&tun_read_buff[0]),size_read)<<"]");
			const int data_route_ttl = 5; // we want to ask others with this TTL to route data sent actually by our programs

			c_haship_addr src_hip, dst_hip;
			std::tie(src_hip, dst_hip) = parse_tun_ip_src_dst(reinterpret_cast<const char *>(&tun_read_buff[0]), size_read);
			// TODO warn if src_hip is not our hip

			if (!addr_is_galaxy(dst_hip)) {
				_dbg3("Got data for strange dst_hip="<<dst_hip);
				continue; // !
			}

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
				_info("Using CT tunnel to send our own data");
				auto & ct = * find_tunnel->second;
				antinet_crypto::t_crypto_nonce nonce_used;

				assert(size_read <= tun_read_buff.size() && size_read >= g_tuntap::header_position_of_ipv6);
				std::string data_cleartext(reinterpret_cast<char *>(&tun_read_buff[g_tuntap::header_position_of_ipv6]), size_read - g_tuntap::header_position_of_ipv6);
				// clear data == ipv6 packet
				data_cleartext.erase(g_ipv6_rfc::header_position_of_dst, g_haship_addr_size); // remove dst addr from ipv6
				std::string data_encrypted = ct.box_ab(data_cleartext, nonce_used);

				this->route_tun_data_to_its_destination_top(
					e_route_method_from_me,
					data_encrypted.c_str(), data_encrypted.size(), // blob
					src_hip, dst_hip,
					c_routing_manager::c_route_reason( c_haship_addr() , c_routing_manager::e_search_mode_route_own_packet),
					data_route_ttl, nonce_used
				); // push the tunneled data to where they belong
			}
			if (!was_anything_sent_from_TUN) {
				ui::action_info_ok("Ok, we sent a packet of data from our computer through virtual network, sending seems to work.");
				was_anything_sent_from_TUN=true;
			}
		}
		else if(m_event_manager.receive_udp_paket()) { // data incoming on peer (UDP) - will route it or send to our TUN
			anything_happened=true;
			c_ip46_addr sender_pip; // peer-IP of peer who sent it

			size_t size_read = m_udp_device.receive_data(buf, sizeof(buf), sender_pip);
			if (size_read == 0) continue; // XXX ignore empty packets

			_note("UDP Socket read from direct sender_pip = " << sender_pip <<", size " << size_read << " bytes: " << string_as_dbg( string_as_bin(buf,size_read)).get());
			// ------------------------------------

			// parse version and command:
			if (! (size_read >= 2) ) { _warn("INVALIDA DATA, size_read="<<size_read); continue; } // !
			assert( size_read >= 2 ); // buf: reads from position 0..1 are asserted as valid now

			int proto_version = static_cast<int>( static_cast<unsigned char>(buf[0]) );
			// let's assume we will be backward compatible (but this will be not the case untill official stable version probably)
			if (c_protocol::current_version < proto_version) throw std::runtime_error("proto_version too new!");
			c_protocol::t_proto_cmd cmd = int_to_enum<c_protocol::t_proto_cmd>( buf[1] );

			// recognize the peering HIP/CA (cryptoauth is TODO)
			c_haship_addr sender_hip;
			c_peering * sender_as_peering_ptr  = nullptr; // TODO(r)-security review usage of this, and is it needed
			if (! c_protocol::command_is_valid_from_unknown_peer( cmd )) {
				c_peering & sender_as_peering = find_peer_by_sender_peering_addr( sender_pip ); // warn: returned value depends on m_peer[], do not invalidate that!!!
				_info("We recognize the sender, as: " << sender_as_peering);
                sender_as_peering.get_stats().update_read_stats(size_read);
                sender_hip = sender_as_peering.get_hip(); // this is not yet confirmed/authenticated(!)
				sender_as_peering_ptr = & sender_as_peering; // pointer to owned-by-us m_peer[] element. But can be invalidated, use with care! TODO(r) check this TODO(r) cast style
			}
			_info("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ Command: " << enum_to_int(cmd) << " from peering ip = " << sender_pip << " -> peer HIP=" << sender_hip);

			if (cmd == c_protocol::t_proto_cmd::e_proto_cmd_tunneled_data) { // [protocol] tunneled data
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
				_info("Received NONCE=" << antinet_crypto::show_nice_nonce(nonce_used) );
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
					_note("UDP data is addressed to us as finall dst, sending it to TUN (after decryption) blob="<<to_debug(blob));

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
						_note("Using CT tunnel to decrypt data for us");
						auto & ct = * find_tunnel->second;
						auto tundata = ct.unbox_ab( blob , nonce_used );


						_note("<<<====== TUN INPUT: " << to_debug(tundata));
						if (check_ip_protocol(tundata)) {
							// add TUN header
							const unsigned char tun_header[] = {0x00, 0x00, 0x86, 0xDD};
							tundata.insert(g_ipv6_rfc::header_position_of_dst, reinterpret_cast<const char *>(dst_hip.data()), dst_hip.size());

/*							if (!check_packet_destination_address(dst_hip, tundata)) {
								_warn("crypto authentification of destination IP failed");
								throw std::runtime_error("crypto authentification of destination IP failed");
							}*/
							if (!check_packet_source_address(src_hip, tundata)) {
								_warn("crypto authentification of source IP failed");
								throw std::runtime_error("crypto authentification of source IP failed");
							}
							tundata.insert(0, reinterpret_cast<const char*>(tun_header), g_tuntap::header_position_of_ipv6);
							auto write_bytes = m_tun_device.write_to_tun(&tundata[0], tundata.size());
							_assert_throw( (write_bytes == tundata.size()) );

						}
						else
							_warn("IP protocol number " << get_ip_protocol_number(tundata) << " not supported.");

					} // we have CT

					if (!was_anything_sent_to_TUN) {
						ui::action_info_ok("Ok, we received a packet of data through virtual network, receiving seems to work.");
						was_anything_sent_to_TUN=true;
					}
				}
				else
				{ // received data that is addresses to someone else
#if 1
					auto data_route_ttl = requested_ttl - 1;
					const int limit_incoming_ttl = c_protocol::ttl_max_accepted;
					if (data_route_ttl > limit_incoming_ttl) {
						_warn("We were requested to route (data) at high TTL (rude) by peer " << sender_hip <<  " - so reducing it.");
						data_route_ttl=limit_incoming_ttl;
					}

					_info("RRRRRRRRRRRRRRRRRRRRRRRRRRR UDP data is addressed to someone-else as finall dst, ROUTING it, at data_route_ttl="<<data_route_ttl);
					if (sender_as_peering_ptr != nullptr) {
						if (sender_as_peering_ptr->get_limit_points() < 0) {
							_dbg1("drop packet (in ROUTING) because points");
							continue;
						}
						// sender_as_peering_ptr->decrement_limit_points();
					}
					this->route_tun_data_to_its_destination_top(
						e_route_method_default,
						blob.c_str(), blob.size(),
						src_hip, dst_hip,
						c_routing_manager::c_route_reason( src_hip , c_routing_manager::e_search_mode_route_other_packet ),
						data_route_ttl,
						nonce_used // forward the nonce for blob
					); // push the tunneled data to where they belong // reinterpret char-signess
#endif
				}

			} // e_proto_cmd_tunneled_data
			else if (cmd == c_protocol::t_proto_cmd::e_proto_cmd_public_hi) { // [protocol]
				_note("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh --> Command HI received");
				size_t offset1=2; assert( size_read >= offset1); // skip CMD headers (TODO instead use one parser)

				trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_buffer_valid() ,
					buf+offset1 , size_read-offset1);

				// TODONOW: size of pubkey is different, use serialize
				// if (cmd_data.bytes.at(pos1)!=';') _throw_error( std::runtime_error("Invalid protocol format, missing coma") ); // [protocol]
				string_as_bin bin_his_IDC_pub( parser.pop_varstring() ); // PARSE
				string_as_bin bin_his_IDI_pub( parser.pop_varstring() ); // PARSE
				string_as_bin bin_his_IDI_IDC_sig( parser.pop_varstring() ); // PARSE

				_info("We received IDC pubkey=" << to_debug( bin_his_IDC_pub ) );
				_info("We received IDI pubkey=" << to_debug( bin_his_IDI_pub ) );
				_info("We received IDI --> IDC signature=" << to_debug( bin_his_IDI_IDC_sig ) );

			try {
				antinet_crypto::c_multikeys_pub his_IDI;
				his_IDI.load_from_bin(bin_his_IDI_pub.bytes);
				his_IDI.set_ipv6_prefix(m_ipv6_prefix);
				antinet_crypto::c_multisign his_IDI_IDC_sig;
				his_IDI_IDC_sig.load_from_bin(bin_his_IDI_IDC_sig.bytes);
				antinet_crypto::c_multikeys_pub::multi_sign_verify(his_IDI_IDC_sig, bin_his_IDC_pub.bytes, his_IDI);

				{ // add peer
					auto his_pubkey = make_unique<c_haship_pubkey>();
					his_pubkey->load_from_bin( bin_his_IDI_pub.bytes );
					his_pubkey->set_ipv6_prefix(m_ipv6_prefix);
					_info("Parsed pubkey into: " << his_pubkey->to_debug());
					t_peering_reference his_ref( sender_pip , his_pubkey->get_ipv6_string_hexdot() );
					add_peer_append_pubkey( his_ref , std::move( his_pubkey ) );
				}

				{ // add node
					c_haship_pubkey his_pubkey;
					his_pubkey.load_from_bin( bin_his_IDI_pub.bytes );
					his_pubkey.set_ipv6_prefix(m_ipv6_prefix);
					add_tunnel_to_pubkey( his_pubkey );
				}
			} catch (std::invalid_argument &) {
				_warn("Fail to verificate his IDC, probably bad public keys or signatures!!!");
			}
			}
			else if (cmd == c_protocol::t_proto_cmd::e_proto_cmd_findhip_query) { // [protocol]
				_warn("QQQQQQQQQQQQQQQQQQQQQQQ - we are QUERIED to find HIP");
				// [protocol] for search query - format is: HIP_BINARY;TTL_BINARY;
				size_t offset1=2; assert( size_read >= offset1);  string_as_bin cmd_data( buf+offset1 , size_read-offset1); // buf -> bin for comfortable use

				auto pos1 = cmd_data.bytes.find_first_of(';',offset1); // [protocol] size of HIP is dynamic  TODO(r)-ERROR XXX ';' is not escaped! will cause mistaken protocol errors
				decltype (pos1) size_hip = g_haship_addr_size; // possible size of HIP if ipv6
				if ((pos1==string::npos) || (pos1 != size_hip)) _throw_error( std::runtime_error("Invalid protocol format, wrong size of HIP field") );

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
						_note("Searching for the route he asks about");
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
						peer_udp->send_data_udp_cmd(c_protocol::t_proto_cmd::e_proto_cmd_findhip_reply, string_as_bin(data), m_udp_device.get_socket()); // <---
						c_peering & sender_as_peering = find_peer_by_sender_peering_addr( sender_pip ); // warn: returned value depends on m_peer[], do not invalidate that!!!
						_dbg1("send route response to " << sender_pip);
						_dbg1("sender HIP " << sender_as_peering.get_hip());
                        _note("Send the route reply");
					} catch(...) {
						_info("Can not yet reply to that route query.");
						// a background should be running in background usually
					}
				}

			}
			else if (cmd == c_protocol::t_proto_cmd::e_proto_cmd_findhip_reply) { // [protocol]
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
					pubkey.set_ipv6_prefix(m_ipv6_prefix);
					add_tunnel_to_pubkey( pubkey );

					c_routing_manager::c_route_info route_info( sender_hip , given_cost , pubkey );
					_info("rrrrrrrrrrrrrrrrrrr route known thanks to peer help:" << route_info);
					// store it, so that we own this object:
					const auto & route_info_ref_we_own = m_routing_manager.add_route_info_and_return( given_goal_hip , route_info );
					UNUSED(route_info_ref_we_own); // TODO TODONOW and reply to others who asked us
				}
			}
			else {
				_warn("??????????????????? Unknown protocol command, cmd=" << enum_to_int(cmd));
				continue; // skip this packet (main loop)
			}
			// ------------------------------------

		} // event: udp
		else {
			_dbg3("No event/idle");
		} // event: idle / nothing happened


		{ // write periodical stats
			bool doit=1;
			if (time_last_idle != nullptr) {
				if (*time_last_idle >= time_now - idle_banner_frequency) doit=0; // cancel the idle report if it's too soon
			}

			if (doit) {
				time_last_idle = make_unique<decltype(time_now)>( time_now );
				_fact("Status: " << node_title_bar);
				debug_peers();
			}
		}

		}
		catch (tuntap_error_devtun &e) {
			_erro(e.what());
			_warn("Trying restar tun/tap device ...");
			try{
				prepare_socket();
			}catch(ui::exception_error_exit){}
		}catch (std::exception &e) {
			_warn("### !!! ### Parsing network data caused an exception: " << e.what());
		}
		catch (...) {
			_erro("### !!! ### Parsing network data caused unknown exception type.");
		}

// stats-TODO(r) counters
//		int sent=0;
//		counter.tick(sent, std::cout);
//		counter_big.tick(sent, std::cout);
	} // while
}

void c_tunserver::run(int time) {
	_goal(mo_file_reader::gettext("L_starting_TUN"));

	{
		_fact("Will now prepare socket");
		prepare_socket();
		if (!m_option_insecure_cap) {
			my_cap::drop_privileges_after_tuntap(); // [security] ok we're done tuntap
		} else _warn("NOT dropping CAP capability (program options?)");
	}

	{
		if (!m_option_insecure_cap) {
			my_cap::drop_privileges_before_mainloop(); // [security] we do not need special privileges since we enter main loop now
		} else _warn("NOT dropping CAP capability (program options?)");
		_fact("Will now enter main event loop");
		event_loop(time);
		_fact("After main event loop");
	}
}


// ----------------------------------------------------------------------------
// @deprecated
void c_tunserver::program_action_set_IDI(const string & keyname) {
	_note("Action: set IDI");
	_info("Setting the name of IDI key to: " << keyname);
	auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
	auto keys = datastore::get_file_list(keys_path);
	bool found = false;
	for (auto &key_name : keys) {
		//remove .PRV extension
		size_t pos = key_name.find(".PRV");
		std::string act = key_name.substr(0,pos);
		if (keyname == act) {	found = true;	break; }
	}
	if (found == false) {
		_erro("Can't find key (" << keyname << ") in your key list, so can't set it as IDI.");
	}
	_info("Key found ("<< keyname <<") and set as IDI");
	datastore::save_string(e_datastore_galaxy_instalation_key_conf,"IDI", keyname, true);
}

// @deprecated
std::string c_tunserver::program_action_gen_key_simple() {
	const string IDI_name = "IDI";
//	ui::action_info_ok("Generating your new keys.");
        ui::action_info_ok(mo_file_reader::gettext("L_generatin_new_keys"));

	std::vector<std::pair<antinet_crypto::t_crypto_system_type,int>> keys; // list of key types
	keys.emplace_back(std::make_pair(antinet_crypto::t_crypto_system_type_from_string("ed25519"), 1));
	auto output_file = IDI_name;
	generate_crypto::create_keys(output_file, keys, true); // ***
	return IDI_name;
}

// @deprecated
void c_tunserver::program_action_gen_key(const boost::program_options::variables_map & argm) {
	_note("Action: gen key");
	if (!argm.count("key-type")) {
		_throw_error( std::invalid_argument("--key-type option is required for --gen-key") );
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
		generate_crypto::create_keys(output_file, keys, true); // ***
	} else if (argm.count("new-key-file")) {
		output_file = argm["new-key-file"].as<std::string>();
		generate_crypto::create_keys(output_file, keys, false); // ***
	} else {
		_throw_error( std::invalid_argument("--new-key or --new-key-file option is required for --gen-key") );
	}
}
// ----------------------------------------------------------------------------

int c_tunserver::get_my_port() const {
	return m_port;
}

std::string c_tunserver::get_my_reference() const {
	ostringstream oss;
	oss << "./tunserver.elf --peer YOURIP:" << get_my_port() << "-" << get_my_ipv6_nice();
	return oss.str();
}

bool c_tunserver::check_ip_protocol(const std::string& data) const{
	char protocol = get_ip_protocol_number(data);
	if(find(m_supported_ip_protocols.begin(), m_supported_ip_protocols.end(), protocol) != m_supported_ip_protocols.end())
		return true;
	else
		return false;
}

int c_tunserver::get_ip_protocol_number(const std::string& data) const{
	size_t pos_ip_protocol_type = g_ipv6_rfc::header_position_of_ip_protocol_type;
	return data.at(pos_ip_protocol_type);
}

void c_tunserver::enable_remove_peers() {
	_dbg2("enable remove peers");
	enable_remove = true;
}

void c_tunserver::set_remove_peer_tometout(unsigned int timeout_seconds) {
	_info("set peer remove timeout " << timeout_seconds);
	peer_timeout = std::chrono::seconds(timeout_seconds);
}

void c_tunserver::set_prefix_len(int prefix) {
	m_prefix_len = prefix;
}

void c_tunserver::set_prefix(const string &prefix) {
	m_ipv6_prefix = prefix;
}

// ------------------------------------------------------------------


