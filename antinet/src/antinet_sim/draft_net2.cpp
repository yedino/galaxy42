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



/// @brief This function tests the code from this file.
int draft_net2() { // the main function for test
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
	
	t_osi2_data data( std::string("HELLOWORLD") );
	
	_mark("Testing - send data:");
	world.find_object_by_name_as_switch("nodeA").send_data(
		world.find_object_by_name_as_switch("nodeE").get_uuid_any(), 
		data);
	
	
	world.tick(); 
	world.tick(); 
	world.tick(); 
	world.tick(); 
	world.tick(); 
	
	_mark("Testing - show object:");
	_info( world.find_object_by_name_as_switch("swA") );
	
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






