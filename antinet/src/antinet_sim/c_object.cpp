#include "c_object.hpp"
#include "c_geometry.hpp"
#include "c_gui.hpp"
#include "c_world.hpp"
#include "c_allegromisc.hpp"
#include <stdexcept>
#include "c_drawtarget_opengl.hpp"
#include "c_api_tr.hpp"

#include "use_opengl.hpp"

c_object::c_object (string name) : m_name(name), m_animframe(0) { }

void c_object::tick () { }

string c_object::get_name () const { return m_name; }


void c_object::draw_allegro(c_drawtarget &drawtarget, c_layer &layer) {
	_dbg3("Drawing (allegro) object (base method used)");
}

void c_object::draw_opengl(c_drawtarget &drawtarget, c_layer &layer) {
	_dbg3("Drawing (opengl) object (base method used)");
}

// ==================================================================

// @TODO rename to draw_allegro?
void c_wallet::draw (BITMAP *frame, int color, t_pos x, t_pos y) const {
	for (const auto &currency: m_currency) {
		std::string text(currency.first + " " + std::to_string(currency.second));
		textout_ex(frame, font, text.c_str(), x, y, color, 255);
		y -= 10;
	}
}

// ==================================================================

c_entity::c_entity (string name, t_pos x, t_pos y) : c_object(name), m_x(x), m_y(y) { }

/*
void c_entity::draw (const c_drawtarget &drawtarget, c_layer_opengl &layer, int color) const {
{
	m_x , m_y ...
	if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
		triangles ...
		glQuad
	}
	...
}
*/


t_pos c_entity::get_x(){return m_x;}
t_pos c_entity::get_y(){return m_y;}

double c_entity::get_distance (const c_entity &entity) {
	t_geo_point my_point(m_x, m_y);
	t_geo_point remote_point(entity.m_x, entity.m_y);
	return c_geometry::distance(my_point, remote_point);
}


void c_entity::draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any) {


    auto layer =
            dynamic_cast<c_layer_opengl &>
            (layer_any);

    const auto & gui = * drawtarget.m_gui;

    if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
        auto selected_object = gui.m_selected_object.lock();
        auto target_object = gui.m_target;
        auto source_object = gui.m_source;
        //std::cout << "DEBUG3" << std::endl;

        if (this == selected_object.get()) { // if I am the selected object
            //circle(frame, vx, vy, 50 - 5, makecol(255, 128, 32));
            glLineWidth(2.0);  //size of line
            glColor3f(1.0, 1.0, 0.0);
//            glBegin(GL_LINES);
//            glVertex3f(0.0, 0.0, 0.0);
//            glVertex3f(-1, 0, 0);
//            glEnd();
        }

        if (this == target_object.get()) { // if I am the target object
            //circle(frame, vx, vy, 50 - 15, makecol(104, 71, 79));
            glLineWidth(1.5);  //size of line
            glColor3f(0.0, 1.0, 0.0);
//            glBegin(GL_LINES);
//            glVertex3f(0.0, 0.0, 0.0);
//            glVertex3f(-1, 0, 0);
//            glEnd();

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.05*cos(angle);
                float y = 0.1*sin(angle);

                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }

        if (this == source_object.get()) {
            glLineWidth(1.5);  //size of line
            glColor3f(0.0, 1.0, 0.0);

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.05*cos(angle);
                float y = 0.1*sin(angle);

                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }
    }
    if (layer.m_layer_nr == e_layer_nr_gui) {
        // std::cout << "DEBUG4" << std::endl;
        auto selected_object = gui.m_selected_object.lock();
        if (this == selected_object.get()) { // if I am the selected object
            glLineWidth(1.0);
            glColor3f(0.0, 1.0, 1.0);

            glBegin(GL_LINE_LOOP);
            for(float angle=0.0; angle<2*M_PI; angle+=0.1) {
                float x = 0.07*cos(angle);
                float y = 0.14*sin(angle);

                glVertex3f(x,y,0.0f);
            }
            glEnd();
        }
    }
    if (layer.m_layer_nr == e_layer_nr_object) {
        //std::cout << "DEBUG5" << std::endl;
//		line(frame, vx - 2, vy - 2, vx + 2, vy + 2, color);
//		line(frame, vx - 2, vy + 2, vx + 2, vy - 2, color);
//		circle(frame, vx, vy, 10, color);


//        glLineWidth(2.5);  //size of line
//        glColor3f(0.0, 0.0, 1.0);
//        glBegin(GL_LINES);
//        glVertex3f(0.0, 0.0, 0.0);
//        glVertex3f(-1, 0, 0);
//        glEnd();
    }
    if (layer.m_layer_nr == e_layer_nr_object_extra) {
         //std::cout << "DEBUG6" << std::endl;
        //textout_ex(frame, font, m_name.c_str(), vx - 20, vy - 35, color, -1);
        glLineWidth(2.5);  //size of line
        glColor3f(0.0, 0.0, 1.0);
//        glBegin(GL_LINES);
//        glVertex3f(0.0, 0.0, 0.0);
//        glVertex3f(-1, 0, 0);
//        glEnd();
    }
}

void c_entity::draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any) {
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	const auto & gui = * drawtarget.m_gui;
	const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position

	if (layer.m_layer_nr == e_layer_nr_gui_bgr) {
		auto selected_object = gui.m_selected_object.lock();
		auto target_object = gui.m_target;
		auto source_object = gui.m_source;

		if (this == selected_object.get()) { // if I am the selected object
			circle(frame, vx, vy, 50 - 5, makecol(255, 128, 32));
		}

		if (this == target_object.get()) { // if I am the target object
			circle(frame, vx, vy, 50 - 15, makecol(104, 71, 79));
		}

		if (this == source_object.get()) {
			circle(frame, vx, vy, 50 - 15, makecol(246, 83, 86));
		}
	}
	if (layer.m_layer_nr == e_layer_nr_gui) {
		auto selected_object = gui.m_selected_object.lock();
		if (this == selected_object.get()) { // if I am the selected object
			const int r1 = 50 - 8, r2 = 50;
			const auto col1 = makecol(192, 192, 192), col2 = makecol(255, 128, 32);
			int nr = 0;
			for (double angle = 0; angle < M_PI * 2; angle += (M_PI * 2) / 16) {
				++nr;
				line(frame,
				     vx + sin(angle) * r1,
				     vy + cos(angle) * r1, vx + sin(angle) * r2, vy + cos(angle) * r2, (nr % 2 ? col1 : col2));
			}
		}
	}
	
	int color = makecol(255,128,0); // color1
	if (layer.m_layer_nr == e_layer_nr_object) {
		line(frame, vx - 2, vy - 2, vx + 2, vy + 2, color);
		line(frame, vx - 2, vy + 2, vx + 2, vy - 2, color);
		circle(frame, vx, vy, 10, color);
	}
	if (layer.m_layer_nr == e_layer_nr_object_extra) {
		textout_ex(frame, font, m_name.c_str(), vx - 20, vy - 35, color, -1);
	}
}

// ==================================================================

// ==================================================================

// ==================================================================

c_cjddev::c_cjddev (string name,
	t_pos x,
	t_pos y,
#if defined USE_API_TR
	t_cjdaddr address_ipv6) : c_entity(name, x, y), m_my_address(address_ipv6) {
#else
	t_cjdaddr address_ipv6) : c_netdev(name, x, y), m_my_address(address_ipv6) {
#endif

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<long long> dis(1,LONG_LONG_MAX);
	m_dht_addr = dis(gen);			//geting random dht address
	
	std::list<t_cjdaddr> tmp_list;
	tmp_list.push_back(m_my_address);
	m_known_nodes[m_dht_addr] = tmp_list;
}

unsigned int cjddev_detail_next_price (unsigned int current) {
	unsigned int prices[] = {1, 5, 10, 20, 25, 50, 100, 200, 1000, 10000};
	const size_t tab_size = sizeof(prices) / sizeof(prices[0]);
	_obvious(tab_size > 0);
	size_t next_i = 0;
	for (size_t i = 0; i < sizeof(prices) / sizeof(prices[0]); ++i) {
		if (prices[i] == current) {
			next_i = i + 1;
		}
	}
	if (next_i == tab_size)
		next_i = 0;
	_assert((next_i >= 0) && (next_i < tab_size));
	return prices[next_i];
}

void cjddev_detail_next_price_inc (unsigned int &current) {
	current = cjddev_detail_next_price(current);
}

void c_cjddev::add_neighbor (shared_ptr<c_cjddev> neighbor) {
	m_neighbors.insert(pair<t_cjdaddr, weak_ptr<c_cjddev> >(neighbor->m_my_address, neighbor));

	if (m_neighbors_prices.find(neighbor->m_my_address) == m_neighbors_prices.end()) { // new link
		m_neighbors_prices.insert(pair<t_cjdaddr, unsigned int>(neighbor->m_my_address, cjddev_detail_next_price(0)));
	} else {
		cjddev_detail_next_price_inc(m_neighbors_prices.at(neighbor->m_my_address));
	}
}

void c_cjddev::add_neighbor (shared_ptr<c_cjddev> neighbor, unsigned int price) {
	add_neighbor(neighbor);
	m_neighbors_prices.at(neighbor->m_my_address) = price;
}


void c_cjddev::draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any) {
     //auto layer = dynamic_cast<c_layer_opengl>(layer_any);
    auto layer = dynamic_cast<c_layer_opengl&>(layer_any);
    const auto & gui = * drawtarget.m_gui;
    const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
	
	// _info("OpenGL draw");
    /* Move Left 1.5 Units And Into The Screen 6.0 */
    float opengl_x = (vx-0.5*SCREEN_W)/(0.5*SCREEN_W);
    float opengl_y = -(vy-0.5*SCREEN_H)/(0.5*SCREEN_H);
    float m_size = 0.03;
    glLoadIdentity();
    glScalef(1,1,1);
    glTranslatef(opengl_x,opengl_y,0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    //glColor3f(1.0,0.0,0.0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, c_bitmaps::get_instance().m_node->w, c_bitmaps::get_instance().m_node->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture (GL_TEXTURE_2D, c_bitmaps::get_instance().m_node_opengl);   //init a texture
    glBegin( GL_QUADS );                                /* Drawing Using Quads       */
    glTexCoord2f (0, 0);
    glVertex3f(  -1.0f*m_size,  2.0f*m_size,  0.0f );   /* Left top       */
    glTexCoord2f (1, 0);
    glVertex3f( 1.0f*m_size, 2.0f*m_size,  0.0f );      /* Right top      */
    glTexCoord2f (1, 1);
    glVertex3f(  1.0f*m_size, -2.0f*m_size,  0.0f );    /* Right bottom   */
    glTexCoord2f (0, 1);
    glVertex3f( -1.0f*m_size, -2.0f*m_size, 0.0f);      /* Left bottom    */
    glEnd( );                                           /* Finished Drawing The Quads */

    glBindTexture(GL_TEXTURE_2D, 0);   // texture
    glDisable(GL_BLEND);

    //c_entity::draw_opengl(drawtarget, layer_any);
    c_entity::draw_opengl(drawtarget, layer);

////////////////////////////////////////////////////////////////////////////////
    if (layer.m_layer_nr == e_layer_nr_object) {
        //BITMAP *fg1;
        //const char *file1;
        //file1 = "dat/server_48x48.png";
        //set_color_conversion(COLORCONV_NONE);
        //fg1 = load_png(file1, NULL); // TODO: optmize/cache and share across objects

//        set_alpha_blender();
//        draw_trans_sprite(frame, c_bitmaps::get_instance().m_node,
//                          vx - c_bitmaps::get_instance().m_node->w / 2, vy - c_bitmaps::get_instance().m_node->h / 2);
//        if (!m_routing_table.empty()) {
//            circle(frame, vx, vy, 10, color);
//        }

//        glLoadIdentity();
//        glColor3f(0.0,1.0,0.0);
//        glScalef(1,1,1);
//        glPointSize(5);
//        glTranslatef((vx-0.5*SCREEN_W),-(vy-0.5*SCREEN_H),0.0f);
//        glBegin(GL_POINTS);
//        glVertex3f(0.0f,0.0f,0.0f);
//        glEnd();
    }
 ////////////////////////////////////////////////////////////////////////////////

    if ((layer.m_layer_nr == e_layer_nr_route) || (layer.m_layer_nr == e_layer_nr_route_extra)) {

        for (auto neighbor : m_neighbors) {
            bool print_send_message = false;
#if defined USE_API_TR
#else
            if (!m_outbox.empty()) {

                // auto const & = at(0) // @TODO wos

                if (m_outbox.at(0)->m_msg->m_to == neighbor.first) {
                    print_send_message = true;
                }
            }
#endif
            shared_ptr<c_cjddev> neighbor_ptr(neighbor.second.lock());

            if (layer.m_layer_nr == e_layer_nr_route) { // draw the links // XXX
                int price = m_neighbors_prices.at(neighbor.first);
                int thick = 1;
                if (price >= 10) thick = 2;
                if (price >= 20) thick = 3;
                if (price >= 50) thick = 4;
                if (price >= 80) thick = 5;
                if (price >= 100) thick = 6;
                if (price >= 500) thick = 7;
                if (price >= 1000) thick = 8;

                float start_connect_x = (vx-0.5*SCREEN_W)/(0.5*SCREEN_W);
                float start_connect_y = -(vy-0.5*SCREEN_H)/(0.5*SCREEN_H);
                float end_connect_x = ((gui.view_x(neighbor_ptr->m_x))-0.5*SCREEN_W)/(0.5*SCREEN_W);
                float end_connect_y = -((gui.view_y(neighbor_ptr->m_y))-0.5*SCREEN_H)/(0.5*SCREEN_H);

                glLineWidth(thick);
                glColor3f(0.5,0.5,0.5);
                glLoadIdentity();
                glBegin(GL_LINES);
                glVertex3f(start_connect_x,start_connect_y,0.0f);
                glVertex3f(end_connect_x,end_connect_y,0.0f);
                glEnd();
            }
        }
    }
}

void c_cjddev::draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any) {
	auto layer = dynamic_cast<c_layer_allegro&>(layer_any);
	BITMAP *frame = layer.m_frame;

	const auto & gui = * drawtarget.m_gui;
    const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position

	c_entity::draw_allegro(drawtarget, layer);
	int color = makecol(0,0,64); // TODO is this ok?
	textout_ex(frame, font, m_my_address.c_str(), vx - 20, vy - 45, color, -1);

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
        if (!m_routing_table.empty()) {
            circle(frame, vx, vy, 10, color);
        }
	}
	////////////////////////////////////////////////////////////////////

	if ((layer.m_layer_nr == e_layer_nr_route) || (layer.m_layer_nr == e_layer_nr_route_extra)) {
		for (auto neighbor : m_neighbors) {
			bool print_send_message = false;
#if defined USE_API_TR
#else
			if (!m_outbox.empty()) {
				
				// auto const & = at(0) // @TODO wos
				
				if (m_outbox.at(0)->m_msg->m_to == neighbor.first) {
					print_send_message = true;
				}
			}
#endif
			shared_ptr<c_cjddev> neighbor_ptr(neighbor.second.lock());

			//if (layer.m_layer_nr == e_layer_m_outbox.at(0)->m_msg-nr_route) { // draw the links
			if (layer.m_layer_nr == e_layer_nr_route) { // draw the links // XXX

				// the main link line
				//			line(frame, vx, vy, neighbor_ptr->vx, neighbor_ptr->vy, color);
				int price = m_neighbors_prices.at(neighbor.first);
				int thick = 1;
				if (price >= 10) thick = 2;
				if (price >= 20) thick = 3;
				if (price >= 50) thick = 4;
				if (price >= 80) thick = 5;
				if (price >= 100) thick = 6;
				if (price >= 500) thick = 7;
				if (price >= 1000) thick = 8;
				alex_thick_line(frame, vx, vy, gui.view_x(neighbor_ptr->m_x), gui.view_y(neighbor_ptr->m_y), thick - 1, color);

				if (print_send_message) {
					//line(frame, vx, vy, neighbor_ptr->vx, neighbor_ptr->vy, makecol(255, 0, 0));
					t_geo_point send_piont, receive_point, msg_circle;
					send_piont.x = vx;
					send_piont.y = vy;

					receive_point.x = gui.view_x(neighbor_ptr->m_x);
					receive_point.y = gui.view_y(neighbor_ptr->m_y);
					
					auto complete =  static_cast<double>(m_animframe) / static_cast<double>(g_max_anim_frame); // 0.0 - 1.0
					// ( m_outbox.at(0)->m_starttime   -   gui.... world... get_simclock() ) / sendingtime
					// sending time - constant for this card, e.g. 2.0
					msg_circle = c_geometry::point_on_line_between_part(send_piont, receive_point, complete);
#if defined USE_API_TR
#else
					if (m_outbox.at(0)->m_msg->m_logic == e_msgkind_buy_net_inq) {
						draw_trans_sprite(frame, c_bitmaps::get_instance().m_package_green,
						                  msg_circle.x - c_bitmaps::get_instance().m_package_green->w / 2,
						                  msg_circle.y - c_bitmaps::get_instance().m_package_green->h / 2);
						std::string text("looking for " + m_outbox.at(0)->m_msg->m_destination);
						textout_ex(frame, font, text.c_str(), msg_circle.x - 70, msg_circle.y - 15, color, -1);
					} else if (m_outbox.at(0)->m_msg->m_logic == e_msgkind_buy_net_menu) {
						std::shared_ptr<msg_buy_menu> menu_msg(std::dynamic_pointer_cast<msg_buy_menu>(m_outbox.at(0)->m_msg));
						//std::shared_ptr<msg_buy_menu> menu_msg(std::dynamic_pointer_cast<msg_buy_menu>(std::dynamic_pointer_cast<msg_buy>(m_outbox.at(0)->m_msg)));
						draw_trans_sprite(frame, c_bitmaps::get_instance().m_package_blue,
						                  msg_circle.x - c_bitmaps::get_instance().m_package_blue->w / 2,
						                  msg_circle.y - c_bitmaps::get_instance().m_package_blue->h / 2);

						/*std::cout << "data from menu_msg" << std::endl;
						std::cout << menu_msg->m_my_price << std::endl;
						std::cout << menu_msg->m_destination << std::endl;
						std::cout << menu_msg->m_from << std::endl;
						std::cout << menu_msg->m_ID << std::endl;
						std::cout << menu_msg->m_logic << std::endl;
						std::cout << menu_msg->m_to << std::endl;
						std::cout << menu_msg->m_ttl << std::endl;
						std::cout << "end of data from menu_msg" << std::endl;*/
						std::string text(
							menu_msg->m_destination + " found, price = " + std::to_string(menu_msg->m_my_price));
						textout_ex(frame, font, text.c_str(), msg_circle.x - 70, msg_circle.y - 15, color, -1);
					} else if (m_outbox.at(0)->m_msg->m_logic == e_msgkind_data) {
						std::shared_ptr<msg_buy_menu> menu_msg(std::dynamic_pointer_cast<msg_buy_menu>(m_outbox.at(0)->m_msg));
						draw_trans_sprite(frame, c_bitmaps::get_instance().m_package_red,
						                  msg_circle.x - c_bitmaps::get_instance().m_package_blue->w / 2,
						                  msg_circle.y - c_bitmaps::get_instance().m_package_blue->h / 2);
						std::string text("send FTP to " + m_outbox.at(0)->m_msg->m_destination);
						textout_ex(frame, font, text.c_str(), msg_circle.x - 70, msg_circle.y - 15, color, -1);
						std::string price_text(std::dynamic_pointer_cast<msg_use>(m_outbox.at(0)->m_msg)->m_payment.first);
						price_text += " ";
						price_text += std::to_string(std::dynamic_pointer_cast<msg_use>(m_outbox.at(0)->m_msg)->m_payment.second);
						textout_ex(frame, font, price_text.c_str(), msg_circle.x - 70, msg_circle.y - 25, color, -1);
					}
#endif
				}
			}

			// draw the link prices
			if ((layer.m_layer_nr == e_layer_nr_route_extra)) {
				t_geo_point text_pos = c_geometry::point_on_line_between_distance(
					t_geo_point(vx, vy), 
					t_geo_point( gui.view_x(neighbor_ptr->m_x), gui.view_y(neighbor_ptr->m_y)), 
					40
				);
				string price = "$" + std::to_string(m_neighbors_prices.at(neighbor.first));
				textout_ex(frame, font, price.c_str(), text_pos.x, text_pos.y - 10, color, -1);
			}

		}
	}


}

void c_cjddev::receive_message (unique_ptr<c_msgtx> &&message) {
#if defined USE_API_TR
#else
	c_netdev::receive_message(std::move(message));
#endif
}

#if defined USE_API_TR
#else
void c_netdev::receive_message (unique_ptr<c_msgtx> &&message) {
	m_inbox.emplace_back(std::move(message));
}

unique_ptr<c_msgtx> c_netdev::send_message () {
	if (m_outbox.empty()) {
		return nullptr;
	}
	unique_ptr<c_msgtx> ret_value = std::move(m_outbox.at(0));
	m_outbox.erase(m_outbox.begin());
	return ret_value;
}
// ==================================================================

c_netdev::c_netdev (string name, t_pos x, t_pos y) : c_entity(name, x, y) {
}
#endif


t_cjdaddr c_cjddev::get_address () const { return m_my_address; }

vector<t_cjdaddr> c_cjddev::get_neighbors_addresses () const {
	vector<t_cjdaddr> ret_vector;
	for (auto &node : m_neighbors) {
		ret_vector.push_back(node.first);
	}
	return ret_vector;
}

unsigned int c_cjddev::get_price (t_cjdaddr address) const {
	if (m_neighbors_prices.find(address) == m_neighbors_prices.end()) {
		return 0;
	} // TODO
	else {
		return m_neighbors_prices.at(address);
	}
}


int c_cjddev::num_of_wating()
{
   return m_wait_hosts.size();
}

#if defined USE_API_TR
/*

void c_netdev::hw_send(t_nym_id addr,std::string &&serialized_msg) {
	_info("c_netdev::hw_send()");
	_info("serialized_msg size: " << serialized_msg.size());
	m_network.lock()->send_message(addr,std::move(serialized_msg));
}
*/
#endif

void c_cjddev::buy_net (const t_cjdaddr &destination_addr) {
	m_wait_hosts.insert(destination_addr);
	for (auto neighbor : m_neighbors) {

#if defined USE_API_TR
		// XXX test sending pings, rm this
		/*_info("test send ping to " << neighbor.first);
		msg_ping ping_request;
		ping_request.m_from = m_my_address;
		ping_request.m_to = neighbor.first;
		ping_request.m_destination = neighbor.first;
		t_message message;
		message.m_remote_id = ping_request.m_to;
		message.m_data = ping_request.serialize();
		m_raw_outbox.emplace_back(std::move(message));
		*/
		msg_dht_hello m_hello;
		//wylosowac adres
		m_hello.m_home_dht_address = m_dht_addr;
		m_hello.m_from = m_my_address;
		m_hello.m_to = neighbor.first;
		m_hello.m_destination = neighbor.first;


		t_message message;
		message.m_remote_id = m_hello.m_to;
		message.m_data = m_hello.serialize();
		//m_raw_outbox.emplace_back(std::move(message));


		//dokonczyc;
#else
		shared_ptr<c_cjddev> neighbor_ptr(neighbor.second.lock());
		unique_ptr<c_msgtx> inq_msg(new c_msgtx);
		inq_msg->m_msg.reset(new msg_buy_inq);
		inq_msg->m_msg->m_from = m_my_address;
		inq_msg->m_msg->m_to = neighbor.first;
		inq_msg->m_msg->m_destination = destination_addr;
		std::cout << m_my_address << " buy net to " << destination_addr << std::endl;
		m_outbox.emplace_back(std::move(inq_msg));

#endif
	}
}

/// [netlogic] sends packets
bool c_tnetdev::send_ftp_packet (const t_cjdaddr &destination_addr, const std::string &data) {
	std::cout << "routing table (host => next hop)" << std::endl;
	for (auto &host : m_routing_table) {
		std::cout << host.first << " => " << host.second.m_address << " price = " << host.second.m_price << std::endl;
	}
	std::cout << "**********" << std::endl;
	// XXX
	//t_cjdaddr next_hop_address = 0;
	bool destination_in_m_neighbors = false;
	bool destination_in_m_routing_table = false;
	if (m_neighbors.find(destination_addr) != m_neighbors.end()) {
		destination_in_m_neighbors = true;
	} else if (m_routing_table.find(destination_addr) != m_routing_table.end()) {
		std::cout << "found " << destination_addr << std::endl;
		//next_hop_address = route.second.m_address;
		destination_in_m_routing_table = true;
	} else {
		std::cout << "cannot find " << destination_addr << std::endl;
		return false;
	}
	unique_ptr<c_msgtx> ftp_packet = make_unique<c_msgtx>();
	ftp_packet->m_msg.reset(new msg_use_ftp);
	shared_ptr<msg_use_ftp> msg_content(std::dynamic_pointer_cast<msg_use_ftp>(ftp_packet->m_msg)); // TODO msg_use_ftp to msg_use
	msg_content->m_data = data;
	msg_content->m_from = m_my_address;
	msg_content->m_destination = destination_addr;
	const string token_name("TOKEN_A"); // TODO

	if (destination_in_m_neighbors) {
		std::cout << "destination in m_neighbors" << std::endl;
		msg_content->m_payment = std::make_pair(token_name, m_neighbors_prices.at(destination_addr));
		msg_content->m_to = destination_addr;
	} else if (destination_in_m_routing_table) {
		std::cout << "destination in m_routing_table" << std::endl;
		msg_content->m_payment = std::make_pair(token_name, m_routing_table.at(destination_addr).m_price);
		msg_content->m_to = m_routing_table.at(destination_addr).m_address;
	}
	msg_content->m_payment.second -= m_neighbors_prices.at(msg_content->m_to);
	m_wallet.m_currency.at(token_name) -= msg_content->m_payment.second;

	std::cout << "*****ftp packet*****" << std::endl;
	std::cout << "data " << msg_content->m_data << std::endl;
	std::cout << "from " << msg_content->m_from << std::endl;
	std::cout << "to  " << msg_content->m_to << std::endl;
	std::cout << "destination " << msg_content->m_destination << std::endl;
	std::cout << "tokens: " << msg_content->m_payment.first << std::endl;
	std::cout << "price = " << msg_content->m_payment.second << std::endl;
	std::cout << "**********" << std::endl;
#if defined USE_API_TR
#else
	m_outbox.emplace_back(std::move(ftp_packet));
#endif
	return true;
}

void c_cjddev::tick () {
	c_object::tick();
}

bool c_cjddev::send_ftp_packet (const t_cjdaddr &destination_addr, const string &data) {
	return true;
}

// ==================================================================

void c_tnetdev::tick () {
	bool dbg = 1;
	c_cjddev::tick();
#if defined USE_API_TR
//	if(dbg) std::cout << "tick()" << std::endl;
	/*m_network.lock()->tick();
	// process outbox
	if (!m_raw_outbox.empty()) {
		c_api_tr::write_message(std::move(m_raw_outbox.at(0))); // send message using c_network
		m_raw_outbox.erase(m_raw_outbox.begin());
	}

	// process inbox
	if (!m_raw_inbox.empty()) {
		std::shared_ptr<msgcjd> input_msg; ///< message from inbox
		input_msg->deserialize(m_raw_inbox.at(0).m_data);
		m_raw_inbox.erase(m_raw_inbox.begin());
		
		if (input_msg->m_logic == e_msgkind_ping_request) {
			if(dbg) _note("send response ping to " << input_msg->m_from);
			std::shared_ptr<msg_ping> ping_msg(std::dynamic_pointer_cast<msg_ping>(input_msg));
			msg_ping_response response;
			response.m_ttl = ping_msg->m_ttl;
			response.m_to = ping_msg->m_from;
			response.m_from = ping_msg->m_to;
			response.m_logic = e_msgkind_ping_response;
			response.m_ID = ping_msg->m_ID;
			response.m_ping_time = get_distance(*std::dynamic_pointer_cast<c_entity>(m_neighbors.at(ping_msg->m_from).lock())); ///< get distance to ping source
			
			t_message out_message;
			out_message.m_remote_id = response.m_to;
			out_message.m_data = response.serialize();
			m_raw_outbox.emplace_back(std::move(out_message));
		}
		else if (input_msg->m_logic == e_msgkind_dht_hello) { ///< response for dht hello
			std::shared_ptr<msg_dht_hello> dht_msg(std::dynamic_pointer_cast<msg_dht_hello>(input_msg));
			if (dht_msg->m_direction == true) {
				if(dbg) _note("get msg hello from " << input_msg->m_from);
				for (auto &tmp : m_known_nodes) {
					std::list<t_cjdaddr> tmp_list;
					tmp_list.push_back(m_my_address);
					tmp_list.insert(tmp_list.end(), tmp.second.begin(), tmp.second.end());
					//m_known_nodes[tmp.first] = dht_msg->m_known_nodes.second; ///< get list from packet
				}
				msg_dht_hello response;
				response.m_home_dht_address = m_dht_addr;
				response.m_from = m_my_address;
				response.m_to = dht_msg->m_from;
				response.m_destination = dht_msg->m_from;
				response.m_known_nodes = m_known_nodes;
				response.m_direction = false;
				
				t_message message;
				message.m_remote_id = response.m_to;
				message.m_data = response.serialize();
				m_raw_outbox.emplace_back(std::move(message));
			}
			else { ///< dht_msg == response fir hello
				if(dbg) _note("get response for dht hello from " << input_msg->m_from);
			}
			
		}
	}*/
#else

	// this should be in a loop to send faster - many packets at once (once full algorithm is implemented)

	// THIS IS JUST A SIMPLE TEST!!! with very expensive full search.
	// we will be implementing a DHT layer next :)

	// process outbox
	// there is something to send in the network from us
	// so send it.
	if (!m_outbox.empty()) { // send outgoing messages to our peers
		if (dbg) { std::cout << "send message from " << m_outbox.at(0)->m_msg->m_from << " to " << m_outbox.at(0)->m_msg->m_to << std::endl; }

		auto &msg = m_outbox.at(0); // the message (processed now)
		auto &msg_recipient = msg->m_msg->m_to; // the recipint address

		shared_ptr<c_cjddev> neighbor_ptr; // to whom should we *directly* send the packet now
		// make decision to whom send now directly:
		if (m_neighbors.find(msg_recipient) != m_neighbors.end()) { // if this recipint is our direct peer then:
			std::cout << "send packet to neighbor" << std::endl;
			neighbor_ptr = m_neighbors.at( msg_recipient  ).lock(); // send to him obviously
		} else if (m_routing_table.find(m_outbox.at(0)->m_msg->m_to) != m_routing_table.end()) {
			std::cout << "send packet using routing table" << std::endl;
			std::cout << "packet to = " << m_outbox.at(0)->m_msg->m_to << std::endl;
			std::cout << "packet dest = " << m_outbox.at(0)->m_msg->m_destination << std::endl;

            auto &next_hop_address = m_routing_table.at(msg_recipient).m_address; // the next hop that we think is a good path there
			neighbor_ptr = m_neighbors.at(next_hop_address).lock();
		}
		neighbor_ptr->receive_message(std::move(m_outbox.at(0))); // push the message to the selected hop
		m_outbox.erase(m_outbox.begin()); // remove this message
	}

	// process inbox
	if (!m_inbox.empty()) {

//        auto &msg = m_inbox.at(0)->m_msg->m_ttl;

		if (m_inbox.at(0)->m_msg->m_ttl == 0) {
			// TODO
		}

		--m_inbox.at(0)->m_msg->m_ttl;
		if (m_inbox.at(0)->m_msg->m_logic == e_msgkind_buy_net_inq) { // e_msgkind_buy_net_inq in inbox
			//std::cout << "(" << m_my_address << ") " << "recv msg_buy_inq"  << std::endl;
			shared_ptr<msg_buy_inq> msg_ptr(std::dynamic_pointer_cast<msg_buy_inq>(m_inbox.at(0)->m_msg));
			m_response_nodes.insert(pair<t_ID, t_cjdaddr>(msg_ptr->m_ID, msg_ptr->m_from));
			if (m_routing_table.find(msg_ptr->m_destination) != m_routing_table.end()) { // host in routing table
				std::cout << "(" << m_my_address << ") " << "found in routing table" << std::endl;
				unique_ptr<c_msgtx> message_resp_ptr(new c_msgtx);
				shared_ptr<msg_buy_menu> msg_content_ptr(new msg_buy_menu);
				message_resp_ptr->m_msg = msg_content_ptr;
				msg_content_ptr->m_my_price = m_routing_table.at(msg_ptr->m_destination).m_price;
				message_resp_ptr->m_msg->m_from = m_my_address;
				message_resp_ptr->m_msg->m_to = msg_ptr->m_from;
				message_resp_ptr->m_msg->m_destination = msg_ptr->m_destination;
				message_resp_ptr->m_msg->m_ID = msg_ptr->m_ID;
				m_outbox.emplace_back(std::move(message_resp_ptr));
			} else if (m_neighbors.find(msg_ptr->m_destination) != m_neighbors.end()) { // my neighbor == destination
				//std::cout << "(" << m_my_address << ") " << msg_ptr->m_destination << " is my neighbor" << std::endl;
				unique_ptr<c_msgtx> message_resp_ptr(new c_msgtx);
				shared_ptr<msg_buy_menu> msg_content_ptr(new msg_buy_menu);
				message_resp_ptr->m_msg = msg_content_ptr;
				msg_content_ptr->m_my_price = m_neighbors_prices.at(msg_ptr->m_destination); // this is the price - directly from the neighbor
				message_resp_ptr->m_msg->m_from = m_my_address;
				message_resp_ptr->m_msg->m_to = msg_ptr->m_from;
				message_resp_ptr->m_msg->m_destination = msg_ptr->m_destination;
				message_resp_ptr->m_msg->m_ID = msg_ptr->m_ID;
				m_outbox.emplace_back(std::move(message_resp_ptr));
			} else { // my neighbor != destination
				bool break_loop = false;
				if (m_wait_hosts.find(msg_ptr->m_destination) != m_wait_hosts.end()) { // I'm waiting for msg_ptr->m_destination
					m_oldbox.emplace_back(std::move(m_inbox.at(0)));
					break_loop = true;
				}
				m_wait_hosts.insert(msg_ptr->m_destination);
				for (auto neighbor : m_neighbors) { // send msg_buy_inq to my neighbor (except msg_ptr->m_from)
					if (break_loop) {
						//std::cout << "break loop" << std::endl;
						break;
					}
					if (msg_ptr->m_from == neighbor.first) {
						continue;
					}
					// send to this neighbor a question for price
					shared_ptr<msg_buy_inq> message_content_ptr(new msg_buy_inq); // this is the same question, he will reply his price - from his POV
					// fill msg
					message_content_ptr->m_from = m_my_address;
					message_content_ptr->m_to = neighbor.first;
					message_content_ptr->m_ttl = msg_ptr->m_ttl;
					message_content_ptr->m_destination = msg_ptr->m_destination;
					message_content_ptr->m_ID = msg_ptr->m_ID; // copy msg
					unique_ptr<c_msgtx> message_resp(new c_msgtx);
					message_resp->m_msg = message_content_ptr;
					m_outbox.emplace_back(std::move(message_resp));
				}
			}
		} else if (m_inbox.at(0)->m_msg->m_logic == e_msgkind_buy_net_menu) {
			//std::cout << "(" << m_my_address << ") " << "recv msg_buy_menu"  << std::endl;
			t_cjdaddr inbox_destination = m_inbox.at(0)->m_msg->m_destination;
			m_wait_hosts.erase(inbox_destination);
			s_remote_host remote_host;
			remote_host.m_address = m_inbox.at(0)->m_msg->m_from;
			remote_host.m_price = std::dynamic_pointer_cast<msg_buy_menu>(m_inbox.at(0)->m_msg)->m_my_price +
			                      m_neighbors_prices.at(m_inbox.at(0)->m_msg->m_from);
			if (m_routing_table.find(inbox_destination) != m_routing_table.end() &&
			    m_routing_table.at(inbox_destination).m_price > remote_host.m_price) {
				m_routing_table.at(inbox_destination).m_address = remote_host.m_address;
				m_routing_table.at(inbox_destination).m_price = remote_host.m_price;
				std::cout << "!!! NEW inserterd address :" << inbox_destination << "NEW Price : " << remote_host.m_price <<
				std::endl;
			} else {
				m_routing_table.insert(pair<t_cjdaddr, s_remote_host>(inbox_destination, remote_host));
			}

			m_neighbors_prices.insert(pair<t_cjdaddr, unsigned int>(remote_host.m_address, remote_host.m_price));
			std::cout << m_my_address << " instrt " << inbox_destination << " => " << remote_host.m_address <<
			" to m_routing_table" << std::endl;
			if (m_response_nodes.find(m_inbox.at(0)->m_msg->m_ID) != m_response_nodes.end()) { // send response
				shared_ptr<msg_buy_menu> resp_msg_content(new msg_buy_menu);
				unique_ptr<c_msgtx> resp_msg(new c_msgtx);
				resp_msg->m_msg = resp_msg_content;
				resp_msg_content->m_from = m_my_address;
				resp_msg_content->m_to = m_response_nodes.at(m_inbox.at(0)->m_msg->m_ID);
				resp_msg_content->m_destination = inbox_destination;
				resp_msg_content->m_ID = m_inbox.at(0)->m_msg->m_ID;
				resp_msg_content->m_my_price = remote_host.m_price;
				m_outbox.emplace_back(std::move(resp_msg));
			}
		}

		else if (m_inbox.at(0)->m_msg->m_logic == e_msgkind_data) { // normal data packet
			shared_ptr<msg_use> msg = std::dynamic_pointer_cast<msg_use>(m_inbox.at(0)->m_msg);
			if (msg->m_destination == m_my_address) { // message to me
				std::cout << m_my_address << " message data = " << msg->m_data << std::endl;
			} else {
				unique_ptr<c_msgtx> out_msg(new c_msgtx);
				out_msg->m_msg.reset(new msg_use);
				shared_ptr<msg_use> msg_content(std::dynamic_pointer_cast<msg_use>(out_msg->m_msg));
				msg_content->m_ttl = m_inbox.at(0)->m_msg->m_ttl;
				msg_content->m_destination = m_inbox.at(0)->m_msg->m_destination;
				msg_content->m_data = std::move(std::dynamic_pointer_cast<msg_use>(m_inbox.at(0)->m_msg)->m_data); // move string
				msg_content->m_from = m_my_address;
				msg_content->m_payment = std::move(std::dynamic_pointer_cast<msg_use>(m_inbox.at(0)->m_msg)->m_payment);
				if (m_neighbors.find(m_inbox.at(0)->m_msg->m_destination) != m_neighbors.end()) { // my neighbor == destination
					msg_content->m_to = m_inbox.at(0)->m_msg->m_destination;
				} else if (m_routing_table.find(m_inbox.at(0)->m_msg->m_destination) !=
				           m_routing_table.end()) { // destination in rputing table
					msg_content->m_to = m_routing_table.at(m_inbox.at(0)->m_msg->m_destination).m_address;
				} else {
					throw std::runtime_error("destination not found");
				}

				// get money from msg
				msg_content->m_payment.second -= m_neighbors_prices.at(msg_content->m_to);
				m_wallet.m_currency.at(msg_content->m_payment.first) += m_neighbors_prices.at(msg_content->m_to);
				m_outbox.emplace_back(std::move(out_msg));
			}
		}

		else { // other messages
			/*if (m_my_address == m_inbox.at(0)->m_msg->m_route.back()) { // I'm receiver
				// TODO
			}
			else {
				vector <t_cjdaddr>::iterator it;
				it = std::find(m_inbox.at(0)->m_msg->m_route.begin(), m_inbox.at(0)->m_msg->m_route.begin(), m_my_address);
				it++;
				unique_ptr<c_msgtx> next_msg(std::move(m_inbox.at(0)));
				next_msg->m_msg->m_from = m_my_address;
				next_msg->m_msg->m_to = *it;
				m_outbox.emplace_back(std::move(next_msg));
			}*/
		}
		m_inbox.erase(m_inbox.begin()); // processed it, can remove here
	}

	if (!m_oldbox.empty()) {
		for (std::vector<unique_ptr<c_msgtx> >::iterator it = m_oldbox.begin(); it < m_oldbox.end(); it++) {
			//for (auto &msgtx : m_oldbox) {
			if (m_routing_table.find((*it)->m_msg->m_destination) !=
			    m_routing_table.end()) { // found destination in routing table
				shared_ptr<msg_buy_inq> msg_ptr(std::dynamic_pointer_cast<msg_buy_inq>((*it)->m_msg));
				unique_ptr<c_msgtx> message_resp_ptr(new c_msgtx);
				shared_ptr<msg_buy_menu> msg_content_ptr(new msg_buy_menu);
				message_resp_ptr->m_msg = msg_content_ptr;
				msg_content_ptr->m_my_price = m_routing_table.at(msg_ptr->m_destination).m_price;
				if (msg_content_ptr->m_my_price == 0) {
					std::cout << "price == 0, oldbox" << std::endl;
				}
				message_resp_ptr->m_msg->m_from = m_my_address;
				message_resp_ptr->m_msg->m_to = msg_ptr->m_from;
				message_resp_ptr->m_msg->m_destination = msg_ptr->m_destination;
				message_resp_ptr->m_msg->m_ID = msg_ptr->m_ID;
				m_outbox.emplace_back(std::move(message_resp_ptr));
				m_oldbox.erase(it);
			}
		}
	}
#endif // defined USE_API_TR
}
// ==================================================================

c_tnetdev::c_tnetdev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6) : c_cjddev(name, x, y, address_ipv6) {
	std::cout << "insert into m_wallet" << std::endl;
	unsigned int a_tokens = std::rand() % 1000 + 1;
	std::cout << "TOKEN_A " << a_tokens << std::endl;
	m_wallet.m_currency.insert(pair<std::string, unsigned int>("TOKEN_A", a_tokens));
	m_wallet.m_currency.insert(pair<std::string, unsigned int>("TOKEN_B", 1000 - a_tokens));
}


void c_tnetdev::draw_opengl(c_drawtarget &drawtarget, c_layer &layer_any) {
	// auto layer = dynamic_cast<c_layer_opengl>(layer_any);
    auto layer = dynamic_cast<c_layer_opengl &>(layer_any);
    //const auto & gui = * drawtarget.m_gui;
    //const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position

    c_cjddev::draw_opengl(drawtarget, layer);
}

void c_tnetdev::draw_allegro(c_drawtarget &drawtarget, c_layer &layer_any) {
	auto layer = dynamic_cast<c_layer_allegro &>(layer_any);
	BITMAP *frame = layer.m_frame;
	const auto & gui = * drawtarget.m_gui;
	const int vx = gui.view_x(m_x), vy = gui.view_y(m_y); // position in viewport - because camera position
	
	
	int color = makecol(255,0,255);
	c_cjddev::draw_allegro(drawtarget, layer);
	assert(!m_wallet.m_currency.empty());// return;

	auto color_wallet = makecol(64,240,255);
	m_wallet.draw(frame, color_wallet, vx - 20, vy - 55);
}

void c_cjddev::remove_neighbor (shared_ptr<c_cjddev> neighbor) {
	remove_neighbor(neighbor->m_my_address);
}

void c_cjddev::remove_neighbor (t_cjdaddr address) {
	if (m_neighbors.find(address) != m_neighbors.end())
		m_neighbors.erase(address);
}

vector<shared_ptr<c_cjddev>> c_cjddev::get_neighbors () const {
	vector<shared_ptr<c_cjddev>> ret_vector;
	for (auto &node : m_neighbors) {
		ret_vector.push_back(node.second.lock());
	}
	return ret_vector;
}


// ==================================================================

c_userdev::c_userdev (string name, t_pos x, t_pos y, t_cjdaddr address_ipv6) : c_tnetdev(name, x, y, address_ipv6) {
}

// ==================================================================

