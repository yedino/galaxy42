#include "osi2.hpp"
#include "c_node.hpp"

c_osi2_cable_direct::c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b) 
  : m_endpoint( {a,b} )
{
}

/*
void c_osi2_cable_direct::draw_allegro (c_drawtarget &drawtarget, c_layer &layer_any) const {
	auto layer = dynamic_cast<c_layer_allegro&>(layer_any);
	BITMAP *frame = layer.m_frame;
	int color = makecol(255,128,0);
	t_pos x1 = m_endpoint.at(0).get().get_my_switch().get_x();
	t_pos y1 = m_endpoint.at(0).get().get_my_switch().get_y();
	t_pos x2 = m_endpoint.at(1).get().get_my_switch().get_x();
	t_pos y2 = m_endpoint.at(1).get().get_my_switch().get_y();
	line(frame, x1, y1, x2, y2, color);
}
*/

////////////////////////////////////////////////////////////////////

c_osi2_cable_direct_plug::c_osi2_cable_direct_plug(c_osi2_cable_direct &cable)
  : m_cable(cable)
{
	
}

std::array< std::reference_wrapper<c_osi2_nic>, 2 > c_osi2_cable_direct::get_endpoints() const {
	return m_endpoint;
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

bool c_osi2_nic::empty_outbox() const {
	return m_outbox.empty();
}


long int c_osi2_nic::get_serial_number() const {
	return m_nr;
}

c_osi2_switch &c_osi2_nic::get_my_switch() const {
	return m_switch;
}

t_osi3_uuid c_osi2_nic::get_uuid() const
{
	return m_osi3_uuid;
}


////////////////////////////////////////////////////////////////////



std::ostream &operator<<(std::ostream &os, const t_osi3_packet &pck)
{
	const size_t preview_size = 20;
	const auto data_len = pck.m_data.size();
	os << "[src=" << pck.m_src
	   <<" to dst=" << pck.m_dst
	   << " data size=" << data_len
	   << " '"<<pck.m_data.substr(0,preview_size)  // start the data
	   << ( data_len>preview_size ? "'... " : "'"  )  // end, mark if was truncated
	   << "]";
	return os;
}
