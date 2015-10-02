#include "libs1.hpp"
#include "c_node.hpp"
#include "osi2.hpp"
#include "c_networld.hpp"

/***

@file Draft for new network simulation system

Again network simualtion refactoring, after all it's crucial thing of this project.
Yo Dawg I herd you like refactoring?

Network simulation:
- We have "switches" that are actually also routers, that always route data to destination UUID address (that is
like an IP address abstraction).
- OSI_2: switches/routers with direct links in LAN, switch congestion
- OSI_3: routing data to the UUID destination (over LAN/ICANN internet) - routing the CJDNS peers
- CJDNS: build DHT, find CJDNS path with directors (using ISO_3)

OSI_2 routing simulation, uses:
- e.g. simple Dijkstra
- is NOT guaranteed to work if there are OSI 2 cables paralell connections (two 
direct connections exist between switch A and switch B) - TODO add detection of this

Implement:
- switches/NIC buffers can fill up and slow down or drop packets
- switches have speed limit of how many packets they can transport in a tick

Skip:
- We skip IP/MAC addressing, we just emulate it with giving NIC cards UUID addresses that are some kind of IP,
to not have to implement NAT and all

- We skip BGP simulation, we just assume data can get from any main ISP to any other main ISP node
(adding some speed limits on each ISP).
- We skip NAT simulation, we assume each station has a public IP (even if closely connected in one LAN)

=== Network Use ===
Send data:
c_osi2_nic::add_to_nic_outbox() is a low-level send of a t_osi3_packet (src, dst, data) 
c_node::send_osi3_data_to_dst() is the high level send of data, to selected dst
c_node::send_osi3_data_to_name() is the high level send of data, where dst is given by object name

*/




/*** 
@brief the entire world of networking
*/

typedef union {
	unsigned char as_ipv4[4]; ///< the 4 octets of ipv4
	unsigned char as_ipv6[16]; ///< the 16 octets of ipv6
} t_osi3_ip ;



// Not doing this after all, too much details - using instead the t_osi3_uuid as "address"
#if 0

/***
 * Select the type of OSI3 address, is it IPv4 or IPv6
 */
typedef enum { e_osi3_addrtype_ipv4, e_osi3_addrtype_ipv6 } t_osi3_addrtype;

struct t_osi3_addr {
	t_osi3_ip ip;
	t_osi3_addrtype type;	
};

#endif

/*
class c_osi2_switch;
class c_osi2_cable_direct; 
class c_osi2_nic;

class c_networld;

class c_node;
*/


typedef float t_netspeed_bps;
typedef float t_netsize_fraction; // fraction of packet in transfer

size_t draft_net2_testsend(t_clock use_tdelta, t_netspeed_bps use_speed, size_t use_size, t_clock sim_length, int dbg) { 
	_mark("Test with use_tdelta="<<use_tdelta<<" use_speed="<<use_speed<<" use_size="<<use_size);
	
	t_netspeed_bps m_net_speed = use_speed;
	t_netsize_fraction m_net_reminder = 0; // bytes that remain partially accumulated from previous iterations
	
	size_t totall_sent=0;
	t_clock clock = 0;	 // world clock in simulation
	
	size_t c=0; // cycle number
	while (true) { // simulation
		++c;
		if (clock>=sim_length) break; // end
		
		t_clock tdelta = use_tdelta;
		clock += tdelta;
		size_t packet_size = use_size;
		
		t_netsize_fraction bytes_now_part = tdelta * m_net_speed ; // bytes we transfer just now 
		t_netsize_fraction bytes_now_all = bytes_now_part + m_net_reminder; // bytes we can now transfer with reminder
		
		t_netsize_fraction send_bytes = bytes_now_all;
		
		if (dbg) _info("=== clock="<<clock<<", cycle="<<c<<", m_net_reminder=" << m_net_reminder
		               << " bytes_now_all=" << bytes_now_all );
		while (send_bytes > packet_size) { // send a packet
			if (dbg) _info("*** SENDING at clock="<<clock<<", cycle="<<c<<", m_net_reminder=" << m_net_reminder);
			totall_sent += packet_size;
			send_bytes -= packet_size;
		}
		
		if (1) { // there is any more data waiting to be sent
			if (dbg) _info("Will send reminder to send_bytes="<<send_bytes);
			m_net_reminder = send_bytes; // "start" sending remaining data - in future
		}
		
		// TODO account for in progress the reminder of time in this cycle
	}
	return totall_sent;
}
	
	
	
bool draft_net2_testsend_speeds(
		vector<t_clock> tab_tdelta, vector<t_netspeed_bps> tab_speed, vector<size_t> tab_size, int dbg
		,t_clock sim_length
	)
{ 
	_mark("Starting test " <<__FUNCTION__);
	
	vector<string> error_tab;
	
	std::ostringstream oss_main;
	for(auto use_tdelta : tab_tdelta) {
		for(auto use_speed : tab_speed) {
			for(auto use_size : tab_size) {
				size_t total = draft_net2_testsend(use_tdelta, use_speed, use_size, sim_length, dbg>=2);
				std::ostringstream oss;
				oss << "tdelta="<<use_tdelta<<" use_speed="<<use_speed<<" use_size="<<use_size<<" : ";
				double avg_speed = total / sim_length;
				double error = std::fabs( (avg_speed - use_speed) / use_speed );
				oss << " avg="<<avg_speed<<" bps, error="<<error;
				string msg = oss.str();
				
				if (error > 0.03) error_tab.push_back(msg);
				
				oss_main << msg << "\n";
				if (dbg >= 2) _info( msg );
			}
		}
	}
	_info("Sim results:\n" << oss_main.str());
	
	for (const auto & e : error_tab) _info("Error was: " << e);
	if (error_tab.size()) {
		_info("Found " << error_tab.size() << " errors."); 
		return false;
	} else _info("All in norm :)");
	return true;
}

bool draft_net2_testsend_alltests() {
	vector<t_clock> td_slow( { 1./5,  1./10, 1./20 } ); // time delta for slow networks
	vector<t_clock> td_fast( {        1./10, 1./20, 1./1000 } ); // time delta for fast networks
	
	vector<size_t> mtu_normal( { 500, 576, 1304, 1500 } );
	
	if (! draft_net2_testsend_speeds( td_slow , { 10, 11, 100, 500, 1000, }, { 1, 2, 3, 4, 5, }, 0	 , 30 ) ) return false;
	if (! draft_net2_testsend_speeds( td_slow , { 1, 2, 3, 4, 5 }, { 1, 2, 3, 4, 5, }, 0, 200 ) ) return false;
	if (! draft_net2_testsend_speeds( td_slow , { 1, 2, 3, 4, 5 }, { 1, 10, 30, 50, 51, 100 }, 0, 10000 ) ) return false;
	
	if (! draft_net2_testsend_speeds( td_fast , { 2*1000, 5*1000, 10*1000, 100*1000 }, mtu_normal, 0 , 30 ) ) return false;
	
	if (! draft_net2_testsend_speeds( td_fast , { 150*1000, 200*1000, 1000*1000 }, mtu_normal, 0 , 30 ) ) return false;
	

	return true;
}
	
	
/// @brief This function tests the code from this file.
int draft_net2() { // the main function for test
/*	
	if (! draft_net2_testsend_alltests()) {
		_erro("Unit test failed!");
		return 0;
	}
	return 0; // !
*/	
	_mark("Starting test " <<__FUNCTION__);
	c_world world;
	
	
	world.add_node("nodeA",100,100); // ***
	world.add_node("nodeB",300,100);
	world.add_node("nodeC1",300,300);
	world.add_node("nodeC2",300,320);
	world.add_node("nodeC3",300,340);
	world.add_node("nodeC4",300,360);
	world.add_node("nodeD",150,200);
	world.add_node("nodeE",400,100); // ***
	
	world.add_osi2_switch("swA", 200,100);
	world.add_osi2_switch("swB", 300,100);
	world.add_osi2_switch("swC", 200,300);
	world.add_osi2_switch("swD", 300,100);
	
	_mark("Connecting devices");
	world.connect_network_devices("nodeA","swA", 1);
	world.connect_network_devices("swA","swB", 1);
	world.connect_network_devices("swB","swD", 1);
	world.connect_network_devices("nodeA","nodeD", 1);
	world.connect_network_devices("swA","swC", 1);
	world.connect_network_devices("swC","nodeC1", 1);
	world.connect_network_devices("swC","nodeC2", 1);
	world.connect_network_devices("swC","nodeC3", 1);
	world.connect_network_devices("swC","nodeC4", 1);
	world.connect_network_devices("swD","nodeE", 1); // ***
	
	_mark("Testing - show object:");
	_info( world.find_object_by_name_as_switch("swA") );
	
	_mark("Testing - show object:");
	_info( world.find_object_by_name_as_switch("nodeA") );
	
	_mark("Find route:");
	t_osi2_route_result route = world.route_find_route_between(
		world.find_object_by_name_as_switch("nodeA"),
		world.find_object_by_name_as_switch("nodeE")
	);
	

	t_osi2_data data( std::string("HELLOWORLD") );
	/*
	_mark("Testing - send data:");
	world.find_object_by_name_as_switch("nodeA").send_data(
		world.find_object_by_name_as_switch("nodeE").get_uuid_any(), 
		data);
	*/
	
	c_node &source_node = dynamic_cast<c_node&> (world.find_object_by_name_as_switch("nodeA"));
	source_node.send_osi3_data_to_name("nodeE", std::move(data));
	
	for (int i = 0; i < 10; ++i) {
		world.tick();
	}
	
	
	//_mark("Testing - show object:");
// 	_info( world.find_object_by_name_as_switch("swA") );
	
	/*
	world.print_route_between(
		world.find_object_by_name_as_switch("nodeA"),
		world.find_object_by_name_as_switch("nodeE")
	);
	*/
	
	
#if 0
	world.connect_network_devices("","");
#endif
	
//	world.serialize(std::cout);
	
	
	/***
	 * 
	 * 
	 *  NodeA           SwitchA         SwitchB 
	 *   nic#0 -------> nic#0 --------> nic#0 ----------------> SwitchD 
	 *   nic#1 ---,     nic#1 --,       nic#1 ---------> NodeB  nic#0 -----> NodeE
	 *            |             |
	 *            |             |
	 *            |             |       SwitchC 
	 *            '--> NodeD    `-----> nic#0 -----> NodeC1
	 *                                  nic#1 -----> NodeC2
	 *                                  nic#2 -----> NodeC3
	 *                                  nic#3 -----> NodeC4
	 * 
	 * 
	 */
	
	
	return 0;
}






