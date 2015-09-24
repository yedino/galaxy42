#include "osi2.hpp"

c_osi2_cable_direct::c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b) 
  : m_endpoint( {a,b} )
{
}
////////////////////////////////////////////////////////////////////

c_osi2_cable_direct_plug::c_osi2_cable_direct_plug(c_osi2_cable_direct &cable)
  : m_cable(cable)
{
	
}

////////////////////////////////////////////////////////////////////

long int c_osi2_nic::s_nr = 0;

c_osi2_nic::c_osi2_nic(c_osi2_switch &my_switch)
  :	m_nr( s_nr++ ), m_switch(my_switch)
{
	m_osi3_uuid = my_switch.get_world().generate_osi3_uuid();
	
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

void c_osi2_nic::add_to_outbox (t_osi3_uuid dst, t_osi2_data &&data) {
	t_osi3_packet packet;
	packet.m_data = data; // TODO move, swap?
	packet.m_dst = dst; 
	packet.m_src = m_osi3_uuid; // I am the sender
	
	m_outbox.emplace_back( packet );
}

////////////////////////////////////////////////////////////////////

long int c_osi2_switch::s_nr = 0;


c_osi2_switch::c_osi2_switch(c_world &world)
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
