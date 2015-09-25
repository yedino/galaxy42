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

long int c_osi2_nic::get_serial_number() const {
	return m_nr;
}


////////////////////////////////////////////////////////////////////

