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
		c_node(c_world &world, const string &name, t_pos x, t_pos y);
		c_node(const c_node &) = delete; ///< copy constructor
		c_node& operator = (const c_node &)  = delete;
		c_node(c_node &&) = default; ///< move constructor
		c_node& operator = (c_node &&)  = default;
		
		
		void send_packet(t_osi3_uuid remote_address, std::string &&data);
};

#endif // C_NODE_HPP
