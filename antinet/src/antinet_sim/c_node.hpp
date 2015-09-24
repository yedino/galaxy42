#ifndef C_NODE_HPP
#define C_NODE_HPP

#include "libs1.hpp"
#include "osi2.hpp"

class c_osi2_switch;
class c_osi2_cable_direct; 
class c_osi2_nic;

class c_world;

class c_node : public c_osi2_switch {
	private:	
		// TODO add s_nr m_nr print and operator<< like in others
		
	public:
		c_node(c_world &world);
		
		void send_packet(t_osi3_uuid remote_address, std::string &&data);
};

#endif // C_NODE_HPP
