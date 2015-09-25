#include "c_node.hpp"




long int c_osi2_switch::s_nr = 0;


c_osi2_switch::c_osi2_switch(c_world &world, const string &name, t_pos x, t_pos y)
  : c_entity(name, x, y), m_nr( s_nr ++ ), m_world(world)
{
	
}

void c_osi2_switch::create_nic()
{
	m_nic.push_back( c_osi2_nic(*this) ); // new card, it is plugged into me and added to me
	_info("Creted new NIC card for my node: " << m_nic.back());
}

c_osi2_nic &c_osi2_switch::get_nic(int nr)
{
	return m_nic.at(nr);
}

c_osi2_nic &c_osi2_switch::use_nic(int nr)
{
	while (! ( nr < m_nic.size() ) ) create_nic();
	if (nr < m_nic.size()) return m_nic[nr];
	throw std::runtime_error("Internal error in creating nodes in use_nic"); // assert
}


void c_osi2_switch::connect_with(c_osi2_nic &target, c_world &world)
{
	create_nic(); // create a new NIC card (it will be at end)
	c_osi2_nic & my_new_port = m_nic.back(); // get this new card
	
	// create the cable (it will be owned by the networld world) that connects this target to my new port
	c_osi2_cable_direct cable = world.new_cable_between( target , my_new_port );
	
	// actually plug in the created table to both ends:
	my_new_port.plug_in_cable(cable); 
	target.plug_in_cable(cable);
	
	// as result, the target NIC has access to our NIC and to us, and vice-versa
	
	_dbg2("In " << world << " connected the target " << target << " to my port " << my_new_port );
}

unsigned int c_osi2_switch::get_cost() {
	return m_connect_cost;
}


void c_osi2_switch::print(std::ostream &os) const
{
	os << "[ SWITCH(#"<<m_nr<<")";
	os << " with " << m_nic.size() << " ports";
	os << " ]";
}

c_world &c_osi2_switch::get_world() const
{
	return m_world;
}

std::ostream & operator<<(std::ostream &os, const c_osi2_switch &obj)
{
	obj.print(os);
	return os;
}






/////////////////////////////////////

c_node::c_node(c_world &world, const string &name, t_pos x, t_pos y)
  : c_osi2_switch( world, name, x, y)
{
	
}

void c_node::send_packet (t_osi3_uuid remote_address, std::string &&data) {
	t_osi2_data out_data;
	use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
}
