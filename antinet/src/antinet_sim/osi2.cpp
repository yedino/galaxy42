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

////////////////////////////////////////////////////////////////////

long int c_osi2_switch::s_nr = 0;

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