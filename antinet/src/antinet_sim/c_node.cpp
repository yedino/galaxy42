#include "c_node.hpp"
#include "libs1.hpp"

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


void c_node::send_packet (t_osi3_uuid remote_address, std::string &&data) {
	t_osi2_data out_data;
	use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
}
