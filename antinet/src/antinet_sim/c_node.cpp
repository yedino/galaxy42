#include "c_node.hpp"




long int c_osi2_switch::s_nr = 0;


c_osi2_switch::c_osi2_switch(c_world &world, const string &name, t_pos x, t_pos y)
  : c_entity(name, x, y), m_nr( s_nr ++ ), m_world(world)
{
	
}

bool c_osi2_switch::operator== (const c_osi2_switch &switch_) {
	return switch_.m_nr == this->m_nr;
}

bool c_osi2_switch::operator!= (const c_osi2_switch &switch_) {
	return switch_.m_nr != this->m_nr;
}


void c_osi2_switch::create_nic()
{
	m_nic.push_back( make_unique<c_osi2_nic>(*this) ); // new card, it is plugged into me and added to me
	_info("Creted new NIC card for my node: " << (* m_nic.back()) );
}

c_osi2_nic &c_osi2_switch::get_nic(unsigned int nr)
{
	return * m_nic.at(nr);
}

c_osi2_nic &c_osi2_switch::use_nic(unsigned int nr)
{
	while (! ( nr < m_nic.size() ) ) create_nic();
	if (nr < m_nic.size()) return * m_nic[nr];
	throw std::runtime_error("Internal error in creating nodes in use_nic"); // assert
}

size_t c_osi2_switch::get_last_nic_index() const {
	return m_nic.size() - 1;
}

void c_osi2_switch::send_data(t_osi3_uuid dst, const t_osi2_data &data)
{
	t_osi3_packet pck;
	pck.m_data = data;
	pck.m_dst = dst;
	pck.m_src = 0; // ! unknown yet (will decide when picking the NIC to send through)
	m_outbox.push_back( pck );
	_dbg3("Added to outbox a pck:" << pck << " my outbox size = " << m_outbox.size());
}

t_osi3_uuid c_osi2_switch::get_uuid_any()
{
	return this->use_nic(0).get_uuid(); // take (or first make) my first NIC card, and return it's UUID as mine
}


void c_osi2_switch::connect_with(c_osi2_nic &target, c_world &world, t_osi2_cost cost)
{
	create_nic(); // create a new NIC card (it will be at end)
	c_osi2_nic & my_new_port = * m_nic.back(); // get this new card
	
	// create the cable (it will be owned by the networld world) that connects this target to my new port
	c_osi2_cable_direct & cable = world.new_cable_between( target , my_new_port , cost );
	
	// actually plug in the created table to both ends:
	my_new_port.plug_in_cable(cable); 
	target.plug_in_cable(cable);
	
	// as result, the target NIC has access to our NIC and to us, and vice-versa
	
	_dbg2("In " << world << " connected the target " << target << " to my port " << my_new_port );
}

unsigned int c_osi2_switch::get_cost() {
	return m_connect_cost;
}


void c_osi2_switch::print(std::ostream &os, int level) const
{
	os << "[" <<this <<" SWITCH(#"<<m_nr<<" '"<<m_name<<"')";
	
	if (level>=-1) {
		os << " with " << m_nic.size() << " ports";
		
		if (level>=0) {
			if (m_nic.size()) { // if ther are ports connected to list
				os << ":" << std::endl;
				for (const unique_ptr<c_osi2_nic> & nic_ptr : m_nic) { // TODO nicer syntax?
					c_osi2_nic & nic = * nic_ptr;
					t_osi2_cost cost;
					c_osi2_nic * othernic_ptr = nic.get_connected_card_or_null( cost );
					if (othernic_ptr) {
						os << " ---(cost=" << cost <<")--> " << *othernic_ptr << std::endl;
					}
				} // show NIC cards
			} // any NIC cards?
		} // level>=0
		
	} // level>=-1
	
	os << " ]";
}

std::string c_osi2_switch::print_str(int level) const
{
	std::ostringstream oss;
	print(oss,level);
	return oss.str();
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


void c_osi2_switch::draw_allegro (c_drawtarget &drawtarget, c_layer &layer_any) {
	c_entity::draw_allegro (drawtarget, layer_any);
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	const auto & gui = * drawtarget.m_gui;
	const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
	/// draw connections
	for (auto &nic : m_nic) {
		t_osi2_cost cost;
		c_osi2_nic *remote_nic = nic->get_connected_card_or_null(cost);
		if (remote_nic == nullptr) continue;
		t_pos x2 = gui.view_x(remote_nic->get_my_switch().get_x());
		t_pos y2 = gui.view_y(remote_nic->get_my_switch().get_y());
		line(frame, vx, vy, x2, y2, makecol(255, 128, 32));
	}
	//draw_messages();
}

void c_osi2_switch::draw_messages() const {
	t_geo_point send_piont, receive_point, msg_circle;
	for (auto &nic_ptr : m_nic) {
		if (! (*nic_ptr).empty_outbox()) {
			
		}
	}
}

void c_osi2_switch::logic_tick() {
	/// process all packets
	for (auto &input_packet : m_inbox) {
		_dbg1(m_name << " get packet from " << input_packet.m_src);
		_dbg1(m_name << " input data: " << input_packet.m_data);
	}
	m_inbox.clear();
}

void c_osi2_switch::recv_tick() {
	for (auto &nic : m_nic) {
		t_osi2_cost cost;
		c_osi2_nic * remote_nic = nic->get_connected_card_or_null(cost);
		if (remote_nic == nullptr) continue;
		remote_nic->insert_outbox_to_vector(m_inbox); /// get all packets from remote nic and insert to inbox
	}
}

void c_osi2_switch::send_tick() {
/*
	c_object &dest_switch = m_world.find_object_by_name_as_object(dest_name);
	c_osi2_switch *next_hop = m_world.print_route_between(dynamic_cast<c_object&>(*this), dest_switch);
	if (next_hop == nullptr) {
		_erro("Next hop not foud");
		return;
	}
	for (auto &nic : m_nic) { /// find NIC for next hop
		if (nic->get_my_switch() == *next_hop) {
			nic->add_to_outbox(dynamic_cast<c_osi2_switch&>(dest_switch).get_uuid_any(), std::move(data));
		}
	}
	//use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
*/
	for (auto && pcg:m_outbox){
		auto dest = pcg.m_dst;
		auto data = pcg.m_data;

		c_object &dest_switch = m_world.find_object_by_uuid_as_switch(dest);
		c_osi2_switch *next_hop = m_world.print_route_between(dynamic_cast<c_object&>(*this), dest_switch );		//TODO somehow cash route

		for (auto &nic : m_nic) { /// find NIC for next hop
			if (nic->get_my_switch() == *next_hop) {
				nic->add_to_outbox(dest, std::move(data));
			}
		}
	}
	m_outbox.clear();

}

void c_osi2_switch::send_hello_to_neighbors() {
	_dbg2("send test hello packet to all neighbors");
	/// send test hello packet to all neighbors
	for (auto &nic : m_nic) {
		t_osi2_cost cost;
		c_osi2_nic * remote_nic = nic->get_connected_card_or_null(cost);
		if (remote_nic == nullptr) continue;
		t_osi3_uuid dest_addr = remote_nic->get_uuid(); /// addres of my neighbor
		nic->add_to_outbox(dest_addr, std::string("HELLO"));
	}
}



/////////////////////////////////////

c_node::c_node(c_world &world, const string &name, t_pos x, t_pos y)
  : c_osi2_switch( world, name, x, y)
{
	
}

bool c_node::operator== (const c_node &node) {
	return node.m_nr == m_nr;
}

bool c_node::operator!= (const c_node &node) {
	return node.m_nr != m_nr;
}


void c_node::send_packet (const std::string &dest_name, std::string &&data) {
	c_object &dest_switch = m_world.find_object_by_name_as_object(dest_name);
	c_osi2_switch *next_hop = m_world.print_route_between(dynamic_cast<c_object&>(*this), dest_switch);
	if (next_hop == nullptr) {
		_erro("Next hop not foud");
		return;
	}
	for (auto &nic : m_nic) { /// find NIC for next hop
		if (nic->get_my_switch() == *next_hop) {
			nic->add_to_outbox(dynamic_cast<c_osi2_switch&>(dest_switch).get_uuid_any(), std::move(data));
		}
	}
	//use_nic(0).add_to_outbox(remote_address , std::move(data)); // TODO m_nic index
}

void c_node::draw_allegro (c_drawtarget &drawtarget, c_layer &layer_any) {
	auto layer = dynamic_cast<c_layer_allegro&>(layer_any);
	BITMAP *frame = layer.m_frame;

	const auto & gui = * drawtarget.m_gui;
    const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
	c_osi2_switch::draw_allegro (drawtarget, layer_any);
	auto color = makecol(0,0,64); // TODO is this ok?
	_UNUSED(color);
	////////////////////////////////////////////////////////////////////
	if (layer.m_layer_nr == e_layer_nr_object) {
		//BITMAP *fg1;
		//const char *file1;
		//file1 = "dat/server_48x48.png";
		//set_color_conversion(COLORCONV_NONE);
		//fg1 = load_png(file1, NULL); // TODO: optmize/cache and share across objects

        set_alpha_blender();
        draw_trans_sprite(frame, c_bitmaps::get_instance().m_node,
                          vx - c_bitmaps::get_instance().m_node->w / 2, vy - c_bitmaps::get_instance().m_node->h / 2);
	}
	////////////////////////////////////////////////////////////////////
}
