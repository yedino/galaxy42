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
	c_world world;
	
	
	world.add_node("nodeA",100,100);
	world.add_node("nodeB",300,100);
	world.add_node("nodeC1",300,300);
	world.add_node("nodeC2",300,320);
	world.add_node("nodeC3",300,340);
	world.add_node("nodeC4",300,360);
	world.add_node("nodeD",150,200);
	
	world.add_osi2_switch("swA", 200,100);
	world.add_osi2_switch("swB", 300,100);
	world.add_osi2_switch("swC", 200,300);
	world.add_osi2_switch("swD", 300,100);
	
	world.connect_network_devices("nodeA","swA");
	world.connect_network_devices("swA","swB");
	world.connect_network_devices("swB","swD");
	world.connect_network_devices("nodeA","nodeD");
	world.connect_network_devices("swA","swC");
	world.connect_network_devices("swC","nodeC1");
	world.connect_network_devices("swC","nodeC2");
	world.connect_network_devices("swC","nodeC3");
	world.connect_network_devices("swC","nodeC4");
	
	t_osi2_data data( std::string("HELLOWORLD") );
	
	world.find_object_by_name_as_switch("nodeA").send_data(
		world.find_object_by_name_as_switch("nodeB").get_uuid_any(), 
		data);
	
#if 0
	world.connect_network_devices("","");
#endif
	
//	world.serialize(std::cout);
	
	
	/***
	 * 
	 * 
	 *  NodeA           SwitchA         SwitchB 
	 *   nic#0 -------> nic#0 --------> nic#0 ---------> SwitchD 
	 *   nic#1 ---,     nic#1 --,       nic#1 ---------> NodeB 
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






