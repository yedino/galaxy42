#include "c_node.hpp"
#include "libs1.hpp"

c_node::c_node(c_networld &networld)
  : c_osi2_switch( networld )
{
	
}

void c_node::send_packet (t_osi3_uuid remote_address, std::string &&data) {
	t_osi2_data out_data;
	use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
}
