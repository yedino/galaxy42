#ifndef C_NODE_HPP
#define C_NODE_HPP

#include "libs1.hpp"
#include "osi2.hpp"

class c_osi2_switch;
class c_osi2_cable_direct; 
class c_osi2_nic;

class c_networld;

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

#endif // C_NODE_HPP
