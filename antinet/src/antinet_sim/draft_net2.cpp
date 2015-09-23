#include "libs1.hpp"

/***

@file Draft for new network simulation system

Again network simualtion refactoring, after all it's crucial thing of this project.
Yo Dawg I herd you like refactoring?

Network simulation:
- OSI_2: switches with direct links in LAN, switch congestion
- OSI_3: routing direct data in LAN and over ICANN Internet
- CJDNS: build DHT, find CJDNS path with directors (using ISO_3)

Implement:
- switches/NIC buffers can fill up and slow down or drop packets
- switches have speed limit of how many packets they can transport in a tick


Skip:
- We skip BGP simulation, we just assume data can get from any main ISP to any other main ISP node
(adding some speed limits on each ISP).
- We skip NAT simulation for now, we assume each station has a public IP (even if closely connected in one LAN)

*/

/*** 
@brief the entire world of networking
*/

typedef std::string t_osi2_data; // some kind of packet of data sent over OSI2

typedef union {
	unsigned char as_ipv4[4]; ///< the 4 octets of ipv4
	unsigned char as_ipv6[16]; ///< the 16 octets of ipv6
} t_osi3_ip ;



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

typedef int t_osi3_uuid; ///< unique address that pretends to be some IP and works in "LAN" and over Internet etc.

/***
 * @brief Use this class to generate the unique UUIDs. E.g. a c_networld could have one of them, then the UUIDs
 * are unique in context of one network world.
 */
class c_osi3_uuid_generator {
	private:
		t_osi3_uuid m_last_uuid; ///< the current position of the generator
	public:
			c_osi3_uuid_generator();
			
			t_osi3_uuid generate();
};

class c_osi2_switch;
class c_osi2_cable_direct; 
class c_osi2_nic;

class c_networld;

class c_node;



class c_node {
	private:
		vector<c_osi2_nic> m_nic; ///< my network cards
		
		c_networld &m_networld; ///< my netwok world in which I exist
		
		// TODO add s_nr m_nr print and operator<< like in others
		
	public:
		c_node(c_networld &networld);
		
		void create_nic(); ///< adds one more NIC card
		c_osi2_nic & get_nic(int nr); ///< gets NIC with this number, throws if it does not exist
		c_osi2_nic & use_nic(int nr); ///< gets NIC with this number, can create it (and all other up to that number)
};

/*** 
 * @brief Classical cable in OSI layer 2, connecting 2 end point stations (e.g. computer to computer 
 * or computer to switch, or switch to switch)
 */
class c_osi2_cable_direct {
	private:
		std::array< std::reference_wrapper<c_osi2_nic>, 2 > m_endpoint; ///< array of the 2 endpoints; as reference
//		std::array< c_osi2_nic*, 2 > m_endpoint; ///< array of the 2 endpoints; as reference
	public:
		// 
		// c_osi2_cable_direct(c_osi2_nic *a, c_osi2_nic *b);
		c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b);
};

/***
 * @brief a plug that is connected to a cable; That is needed for C++ reasons,
 * to hold a reference to the cable (the reference can not be re-seated, but
 * entire object of this class can be recreated with ref to other cable when
 * needed) 
 **/
class c_osi2_cable_direct_plug {
	private:
		c_osi2_cable_direct & m_cable;
	public:
		c_osi2_cable_direct_plug(c_osi2_cable_direct & cable);
		friend class c_osi2_nic; ///< friend class, so it can take the .m_cable
};

/***
 * @brief The network card for OSI layer 2.
 * It can store data that is being sent in outbox[]
 */
class c_osi2_nic {
	private:
		std::vector< t_osi2_data > m_outbox; ///< the data that is being sent out in OSI2
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		t_osi3_uuid m_osi3_uuid; ///< the UUID that imitates some unique "IP address" of this card (e.g. like internet IP)
		
		unique_ptr<c_osi2_cable_direct_plug> m_plug; ///< the (plug to the) cable to my physical-peer
		
		// t_osi3_addr m_addr; ///< my address in OSI3 (in LAN, in ICANN internet) // TODO remove
	public:
		c_osi2_nic(c_networld &networld);
		
		void plug_in_cable(c_osi2_cable_direct & cable); ///< attach this cable to me
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_osi2_nic &obj);
};

long int c_osi2_nic::s_nr = 0;

// Classical switch in OSI layer 2
class c_osi2_switch {
	private:
		std::vector<c_osi2_nic> m_nic; ///< all my NIC cards, for all my ports
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
	public:
		c_osi2_switch();
		
		void connect_with(c_osi2_nic &target, c_networld &networld); ///< add port, connect to target, inside networld
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_osi2_switch &obj);
		
};

long int c_osi2_switch::s_nr = 0;

/***
 * @brief one world of the networking, holds (and usually owns) some objects:
 * - should hold the cables (no one holds them since they are in between other objects)
 */
class c_networld {
	private:
		std::vector< c_osi2_cable_direct > m_cable_direct; ///< all the cables that are hold by this networld
		
		static long int s_nr; ///< serial number of this object - the static counter
		long int m_nr; ///< serial number of this object
		
		c_osi3_uuid_generator m_uuid_generator; ///< to generate UUIDs (addresses) in my context
	public:
		c_networld();
		c_osi2_cable_direct& get_new_cable_between(c_osi2_nic &a, c_osi2_nic &b); ///< create a new cable, own it, and return ref to it
		
		t_osi3_uuid generate_osi3_uuid();
		
		void print(std::ostream &os) const;
		friend std::ostream& operator<<(std::ostream &os, const c_networld &obj);
};

long int c_networld::s_nr = 0;

/// @brief This function tests the code from this file.
int draft_net2() { // the main function for test
	c_networld world;
	
	vector<c_node> node;
	node.push_back( c_node(world) );
	
	vector<c_osi2_switch> sw;
	sw.push_back( c_osi2_switch() );
	sw.at(0).connect_with( node.at(0).use_nic(0) , world );
	
	_dbg2( sw.at(0) );
	
	return 0;
}



c_osi2_cable_direct::c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b) 
  : m_endpoint( {a,b} )
{
}

c_osi2_nic::c_osi2_nic(c_networld &networld)
  :	m_nr( s_nr++ )
{
	m_osi3_uuid = networld.generate_osi3_uuid();
}

void c_osi2_nic::plug_in_cable(c_osi2_cable_direct &cable)
{
	m_plug.reset( new c_osi2_cable_direct_plug( cable ) );
}

void c_osi2_nic::print(std::ostream &os) const {
	os << "NIC(#"<<m_nr<<", addr="<<m_osi3_uuid<<")";
}

std::ostream& operator<<(std::ostream &os, const c_osi2_nic &obj) {
	obj.print(os);
	return os;
}


c_osi2_switch::c_osi2_switch()
  : m_nr( s_nr ++ )
{
	
}

void c_osi2_switch::connect_with(c_osi2_nic &target, c_networld &networld)
{
	m_nic.push_back( c_osi2_nic(networld) );
	c_osi2_nic & my_new_port = m_nic.back();
	c_osi2_cable_direct cable = networld.get_new_cable_between( target , my_new_port );
	my_new_port.plug_in_cable(cable);
	target.plug_in_cable(cable);
	_dbg2("In " << networld << " connected the target " << target << " to my port " << my_new_port );
}

void c_osi2_switch::print(std::ostream &os) const
{
	os << "[ SWITCH(#"<<m_nr<<")";
	os << " with " << m_nic.size() << " ports";
	os << " ]";
}

std::ostream & operator<<(std::ostream &os, const c_osi2_switch &obj)
{
	obj.print(os);
	return os;
}

c_networld::c_networld()
	: m_nr( s_nr++ )
{
	
}

c_osi2_cable_direct & c_networld::get_new_cable_between(c_osi2_nic &a, c_osi2_nic &b)
{
	m_cable_direct.emplace_back( a,b );
	return m_cable_direct.back();
}

t_osi3_uuid c_networld::generate_osi3_uuid()
{
	return m_uuid_generator.generate();
}

void c_networld::print(std::ostream &os) const
{
	os << "World(#"<<m_nr<<")";
}

std::ostream & operator<<(std::ostream &os, const c_networld &obj)
{
	obj.print(os);
	return os;
}


c_osi3_uuid_generator::c_osi3_uuid_generator()
  : m_last_uuid(10000)
{
	
}

t_osi3_uuid c_osi3_uuid_generator::generate() 
{
	++m_last_uuid;
	return m_last_uuid;
}


c_osi2_cable_direct_plug::c_osi2_cable_direct_plug(c_osi2_cable_direct &cable)
  : m_cable(cable)
{
	
}

c_node::c_node(c_networld &networld)
  : m_networld( networld )
{
	
}

void c_node::create_nic()
{
	m_nic.push_back( c_osi2_nic(m_networld) );
	_info("Creted new NIC card for my node: " << m_nic.back());
}

c_osi2_nic &c_node::get_nic(int nr)
{
	return m_nic.at(nr);
}

c_osi2_nic &c_node::use_nic(int nr)
{
	while (! ( nr < m_nic.size() ) ) create_nic();
	if (nr < m_nic.size()) return m_nic[nr];
	throw std::runtime_error("Internal error in creating nodes in use_nic"); // assert
}


