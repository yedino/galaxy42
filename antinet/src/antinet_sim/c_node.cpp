#include "c_node.hpp"

c_node::c_node(c_world &world, const string &name, t_pos x, t_pos y)
  : c_osi2_switch( world, name, x, y)
{
	
}

void c_node::send_packet (t_osi3_uuid remote_address, std::string &&data) {
	t_osi2_data out_data;
	use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
}
