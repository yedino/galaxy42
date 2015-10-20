#include "c_node.hpp"

long int c_osi2_switch::s_nr = 0;


c_osi2_switch::c_osi2_switch(c_world &world, const string &name, t_pos x, t_pos y)
  : c_entity(name, x, y), m_nr( s_nr ++ ), m_world(world), m_type(e_switch)
{
	
}

bool c_osi2_switch::operator== (const c_osi2_switch &switch_) {
	if (m_type != switch_.m_type) return true;
	return switch_.m_nr == this->m_nr;
}

bool c_osi2_switch::operator!= (const c_osi2_switch &switch_) {
	if (m_type != switch_.m_type) return false;
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

t_device_type c_osi2_switch::get_type() {
	return m_type;
}

size_t_maybe c_osi2_switch::get_last_nic_index() const {
	auto size = m_nic.size();
	if (size) return m_nic.size() - 1;
	return size_t_invalid();
}

size_t_maybe c_osi2_switch::find_which_nic_goes_to_switch_or_invalid(const c_osi2_switch *obj) {
	t_osi2_cost cost;
	size_t size = m_nic.size();
	for (size_t ix=0; ix<size; ++ix) {
		c_osi2_nic * paired_nic_ptr = m_nic[ix]->get_connected_card_or_null(cost);
		if (paired_nic_ptr) {
			// compare pointers, is this identical mem-address (is it exactly our object)
			if ( & paired_nic_ptr->get_my_switch() == obj ) {
				return ix; // <---
			}
		}

	}
	return size_t_invalid(); // not found
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
        textout_ex(frame, font, (std::to_string(get_uuid_any())).c_str(), vx - 20, vy + 35, makecol(0,0,64), -1);
	}
	draw_messages(drawtarget, layer_any);
	if(!m_draw_outbox.empty() &&
	   layer.m_layer_nr == e_layer_nr_route_activity) {
		draw_packet(drawtarget,layer_any);
	}
}

void c_osi2_switch::draw_messages(c_drawtarget &drawtarget, c_layer &layer_any) const {
	_dbg3("draw message");
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	const auto & gui = * drawtarget.m_gui;
	const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
	t_geo_point send_piont, receive_point, msg_circle;
    //_dbg1(".................VX: " << vx << " ................VY: " << vy);
	for (auto &nic_ptr : m_nic) {
		std::stringstream ss;
		ss << *nic_ptr << std::endl;
		if(m_inbox.size()!=0){
            textout_ex(frame, font, ((std::string)("Data: ")).c_str(), vx+10, vy-20, makecol(255,0,0), -1);
            textout_ex(frame, font, (m_inbox.at(0).m_data).c_str(), vx+10, vy-10, makecol(0,150,255), -1);
            textout_ex(frame, font, ((std::string)("Dest addr: ")).c_str(), vx+10, vy, makecol(255,0,0), -1);
            textout_ex(frame, font, (std::to_string(m_inbox.at(0).m_dst)).c_str(), vx+10, vy+10, makecol(0,150,255), -1);
            textout_ex(frame, font, ((std::string)("Dest src: ")).c_str(), vx+10, vy+20, makecol(255,0,0), -1);
            textout_ex(frame, font, (std::to_string(m_inbox.at(0).m_src)).c_str(), vx+10, vy+30, makecol(0,150,255), -1);
	}
    }
}

void c_osi2_switch::draw_packet(c_drawtarget &drawtarget, c_layer &layer_any) {
	double draw_step = 1./static_cast<double>(g_max_anim_frame);
	const auto & gui = * drawtarget.m_gui;
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	c_osi2_switch &tmp_osi2_switch = m_world.find_object_by_uuid_as_switch(m_draw_outbox.front().first);
	const int this_vx = gui.view_x(m_x), this_vy = gui.view_y(m_y);
	const int next_vx = gui.view_x(tmp_osi2_switch.m_x), next_vy = gui.view_y(tmp_osi2_switch.m_y);
	t_geo_point A(this_vx,this_vy);
	t_geo_point B(next_vx,next_vy);
	t_geo_point between = c_geometry::point_on_line_between_part(A,B,m_draw_outbox.front().second);
	_dbg1("DEBUG<<<<: " << between.x << "  " << between.y);
	_dbg1("Layer_allegro nr: " << layer.m_layer_nr);
	draw_trans_sprite(frame, c_bitmaps::get_instance().m_package_green,
					  between.x - c_bitmaps::get_instance().m_package_green->w / 2,
					  between.y - c_bitmaps::get_instance().m_package_green->h / 2);
	textout_ex(frame, font, (std::to_string(m_draw_outbox.front().second)).c_str(), between.x, between.y-10, makecol(0,15,25), -1);
	if(m_draw_outbox.front().second < 1.) {
		m_draw_outbox.front().second += draw_step;
	} else {
		m_draw_outbox.pop();
	}

}

void c_osi2_switch::logic_tick() {
	/// process all packets
	for (auto &input_packet : m_inbox) {
		//_dbg1(m_name << " get packet from " << input_packet.m_src);
		//_dbg1(m_name << " input data: " << input_packet.m_data);
		m_outbox.push_back(std::move(input_packet));
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
	for (auto & pcg : m_outbox){
		auto const & dest = pcg.m_dst;

		c_osi2_switch &dest_switch = m_world.find_object_by_uuid_as_switch(dest);
		
		c_dijkstry01 dij(*this,dest_switch,m_world);
		std::list<t_osi3_uuid> route_list = dij.get_last_routeList();
		if(route_list.size() >= 2) {
			c_osi2_nic & nic = dij.get_next_nic();
			m_draw_outbox.emplace(std::make_pair<t_osi3_uuid,double>(dij.get_next_uuid(),0.));
			nic.add_to_nic_outbox(std::move( pcg )); // move this packet there
		} else {
			_dbg1("Packet hit detination: ok");
		}

//		size_t nic_ix = m_world.route_next_hop_nic_ix( dynamic_cast<c_object&>(*this), dest_switch );
//		_dbg1("*this sw: " << get_uuid_any() <<"\ndest sw:" << dest_switch.get_uuid_any());

//		if (size_t_is_ok(nic_ix)) {
//			c_osi2_nic & nic = * m_nic.at(nic_ix); // send through this nic
//			std::cout << "old nic adrr: " << &nic << std::endl;
//			nic4.add_to_nic_outbox(std::move( pcg )); // move this packet there
//		} else if(get_uuid_any() == dest_switch.get_uuid_any()) {

//		}
//		else {
//			_warn("Can not find the route between from me " << (*this) << " to dest_switch=" << dest_switch);
//			_dbg1("nic_ix = " << nic_ix);
//		}
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
		// TODO: send it here.
		_NOTREADY_warn();
	}
}

void c_osi2_switch::snd_pgk_test(t_osi3_packet &&packet) {
	m_outbox.push_back(packet);
	_dbg1("******snd_pkg*******only testing**************** ");
	_dbg1("***************************get apcket to " << packet.m_dst);

}

/////////////////////////////////////

c_node::c_node(c_world &world, const string &name, t_pos x, t_pos y)
  : c_osi2_switch( world, name, x, y)
{
	m_type = e_node;
}

bool c_node::operator== (const c_node &node) {
	if (m_type != node.m_type) return false;
	return node.m_nr == m_nr;
}

bool c_node::operator!= (const c_node &node) {
	if (m_type != node.m_type) return true;
	return node.m_nr != m_nr;
}


void c_node::send_osi3_data_to_dst(t_osi3_uuid dst, t_osi2_data &&data) {
	_info("send_osi3_data_to_dst(), send data to " << dst);
	t_osi3_packet packet;
	packet.m_data = std::move(data); // move the data here
	packet.m_dst = dst; 
	packet.m_src = this->get_uuid_any(); // I am the sender. TODO choose the proper card that will be used - routing
	m_outbox.emplace_back( packet ); // to my, switch'es global outbox
}

void c_node::send_osi3_data_to_node(const c_node &dst, std::string &&data)
{
	_NOTREADY();
}
	
void c_node::send_osi3_data_to_name(const std::string &dest_name, std::string &&data)
{
	_note("Sending OSI3 data, to dest_name="<<dest_name<<" data="<<data);
	
	// TODO: this->m_world.find_object_by_name_as_switch(dest_name).get_uuid_reachable_from( this );
	_NOTREADY();
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

void c_node::process_packet (t_osi3_packet &&packet) {
	// TODO!!!
	snd_pgk_test(std::move(packet));
	_dbg1("***************************get apcket from " << packet.m_src);
	_dbg1("***************************data: " << packet.m_data);
}

void c_node::logic_tick() {
	for (auto &input_packet : m_inbox) {
		bool packet_to_me = false;
		for (auto &nic : m_nic) {
			if (nic->get_uuid() == input_packet.m_dst) {
				packet_to_me = true;
				break;
			}
		}
		if (packet_to_me) process_packet(std::move(input_packet));
		else m_outbox.push_back(std::move(input_packet));
	}
	m_inbox.clear();
}
