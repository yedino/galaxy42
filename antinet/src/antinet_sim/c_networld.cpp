#include "c_networld.hpp"

long int c_networld::s_nr = 0;


c_networld::c_networld()
	: m_nr( s_nr++ )
{
	
}

c_osi2_cable_direct & c_networld::get_new_cable_between(c_osi2_nic &a, c_osi2_nic &b)
{
	m_cable_direct.emplace_back( a,b );
	return m_cable_direct.back();
}

t_osi3_uuid c_networld::generate_osi3_uuid()
{
	return m_uuid_generator.generate();
}

void c_networld::generate_next_hop_map() {

}


t_osi3_uuid c_networld::get_next_hop (const t_osi3_uuid &source, const t_osi3_uuid &destination) {
	std::pair < t_osi3_uuid, t_osi3_uuid > path_pair = std::make_pair(source, destination);
	return m_next_hop_map.at(path_pair);
}


void c_networld::print(std::ostream &os) const
{
	os << "World(#"<<m_nr<<")";
}

std::ostream & operator<<(std::ostream &os, const c_networld &obj)
{
	obj.print(os);
	return os;
}