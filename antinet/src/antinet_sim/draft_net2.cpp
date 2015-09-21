#include "libs1.hpp"

/***

@file Draft for new network simulation system

Again network simualtion refactoring, after all it's crucial thing of this project.
Yo Dawg I herd you like refactoring?

Network simulation:
- ISO_2: switches with direct links in LAN, switch congestion
- ISO_3: routing direct data in LAN and over ICANN Internet
- CJDNS: build DHT, find CJDNS path with directors (using ISO_3)

*/

/*** 
@brief the entire world of networking
*/

class c_osi2_switch;
class c_osi2_cable_direct;
class c_network;
class c_node;

class c_node {
	private:
};

/*** 
 * @brief Classical cable in OSI layer 2, connecting 2 end point stations (e.g. computer to computer 
 * or computer to switch, or switch to switch
 */
class c_osi2_cable_direct {
	private:
		std::array<c_node, 2> endpoint;
	public:
		c_osi2_cable_direct(c_node &a, c_node &b);
};

// Classical switch in OSI layer 2
class c_osi2_switch {
	private:
	public:
};

class c_network {
	private:
	public:
};


/// @brief This function tests the code from this file.
int draft_net2() {
	c_network the_network;
	
	_info( sizeof( c_osi2_cable_direct) );
	_info( sizeof( the_network ) );
}



c_osi2_cable_direct::c_osi2_cable_direct(c_node &a, c_node &b) 
  : endpoint( { a , b } )
{
}
