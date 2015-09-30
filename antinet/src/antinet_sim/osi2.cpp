#include "osi2.hpp"
#include "c_node.hpp"

c_osi2_cable_direct::c_osi2_cable_direct(c_osi2_nic &a, c_osi2_nic &b, t_osi2_cost cost) 
  : m_endpoint( {a,b} ), m_cost(cost)
{
	_info("NEW cable, in "<<this<<" I am referencing endpoints: " << &a << " and " << &b << " at cost");
	_info("NEW cable, in "<<this<<" my endpoints are: " << endl << a << endl << " and: " << endl << b );
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
	_info("NEW cable PLUG, in "<<this<<" I will connect cable: " << &cable);
}

std::array< std::reference_wrapper<c_osi2_nic>, 2 > c_osi2_cable_direct::get_endpoints() const {
	return m_endpoint;
}

t_osi2_cost c_osi2_cable_direct::get_cost() const
{
	return m_cost;	
}

c_osi2_nic &c_osi2_cable_direct::get_other_end(const c_osi2_nic &other_then_me)
{
	// is this exactly this one object (not just other one that is the same)
	// compare addresses of references objects
	
	_info("I am the cable in "<<this);
	_info("endpoint 0 address: " << & m_endpoint[0]);
	_info("endpoint 1 address: " << & m_endpoint[1]);
	
	_info("endpoint 0 object: " << m_endpoint[0]);
	_info("endpoint 1 object: " << m_endpoint[1]);
	
	c_osi2_nic & endA = m_endpoint[0];
	c_osi2_nic & endB = m_endpoint[1];
	
	_info("endpoint A object: " << endA);
	_info("endpoint B object: " << endB);
	
	if ( & endA == & other_then_me) {
		if ( & endB == & other_then_me) throw std::runtime_error("Invalid cable, both ends are me"); // assert
		return endB;
	}
	return endA;
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
	_info("nic card "<<this<<" is plugging in cable "<<&cable);
	m_plug.reset( new c_osi2_cable_direct_plug( cable ) );
}

void c_osi2_nic::print(std::ostream &os) const {
	os << "NIC(" << this << " #"<<m_nr<<", addr="<<m_osi3_uuid<<")";
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

c_osi2_nic * c_osi2_nic::get_connected_card_or_null(t_osi2_cost &cost) const
{
	_info("Getting connected card, of me = " << (*this));
	if (!m_plug) {
		_dbg2("... I have no connected card (no plug)");
		return nullptr;
	}
	c_osi2_cable_direct & cable = m_plug->m_cable;
	_dbg2("My plug points to cable " << &cable);
	
	cost = cable.get_cost();
	_dbg2("This cable has cost="<<cost);
	c_osi2_nic * other_nic = & cable.get_other_end(*this);
	_dbg2("This cable shows other_nic = " << other_nic);
	
	return other_nic;
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
