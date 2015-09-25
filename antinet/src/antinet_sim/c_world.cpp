#include "c_world.hpp"

#include "c_drawtarget_opengl.hpp"



long int c_world::s_nr = 0;


c_world::c_world()
	: m_nr( s_nr++ )
{
	
}

c_osi2_cable_direct & c_world::new_cable_between(c_osi2_nic &a, c_osi2_nic &b)
{
	m_cable_direct.emplace_back( a,b );
	return m_cable_direct.back();
}

t_osi3_uuid c_world::generate_osi3_uuid()
{
	return m_uuid_generator.generate();
}

void c_world::print(std::ostream &os) const
	{
	os << "World(#"<<m_nr<<")";
}

std::ostream & operator<<(std::ostream &os, const c_world &obj)
{
	obj.print(os);
	return os;
}




void c_world::add_test () {

	/*m_objects.push_back( make_shared<c_cjddev>("NODE_A", 200, 200, 11111111));
	m_objects.push_back( make_shared<c_tnetdev>("NODE_T_B", 250, 100, 22222222));
	m_objects.push_back( make_shared<c_cjddev>("NODE_C", 150, 90, 33333333));
	m_objects.push_back( make_shared<c_tnetdev>("NODE_T_D", 290, 150, 44444444));
	
	m_objects.push_back( make_shared<c_tnetdev>("NODE_T_A", 700, 700, 55555555));
	m_objects.push_back( make_shared<c_cjddev>("NODE_B", 750, 600, 66666666));
	m_objects.push_back( make_shared<c_tnetdev>("NODE_T_C", 670, 590, 77777777));
	m_objects.push_back( make_shared<c_cjddev>("NODE_D", 730, 650, 88888888));
	
	m_objects.push_back(make_shared<c_userdev>("USER_A", 900, 600, 999999999));
	m_objects.push_back(make_shared<c_userdev>("USER_B", 50, 60,  12546879));
	
	m_objects.push_back( make_shared<c_cjddev>("NODE_E", 330, 450, 10100101));
	m_objects.push_back( make_shared<c_cjddev>("NODE_F", 350, 430, 14565789));
	m_objects.push_back( make_shared<c_cjddev>("NODE_G", 380, 500, 47866545));
	m_objects.push_back( make_shared<c_cjddev>("NODE_T_E", 300, 500, 54878933));
	
	connect_nodes(m_objects.at(1), m_objects.at(2));
	connect_nodes(m_objects.at(6), m_objects.at(7));*/
    //load("layout/current/default.map.txt");
	

/*
	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_1", 200, 200));
	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_2", 250, 100));
	m_objects.emplace_back( make_unique<c_osi2_switch>(*this, "SWITCH_1", 400, 150));
	
	connect_nodes(unique_cast_ref<c_object &>(m_objects.at(1)), unique_cast_ref<c_object &>(m_objects.at(2)));
	*/
}


void c_world::tick () {
	static unsigned int tick_number = 0;
	m_simclock += get_chronon(); // @TODO move the 0.1 speed to a (const?) variable
	
#if defined USE_API_TR
	// @TODO remove deprecated?
	m_network->tick();
#endif
	
	for (auto &obj : m_objects) {
		try{
			obj->tick();
		}catch(...){
			std::cout<<"something goes wrong ..."<<std::endl;
		}
	}
	
	std::cout << "****************END OF TICK (" << tick_number << ")****************" << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(1)); // XXX
	++tick_number;
}

void c_world::draw (c_drawtarget &drawtarget) {
	
	
	if (drawtarget.m_drawtarget_type == e_drawtarget_type_null) {
	} 
	else if (drawtarget.m_drawtarget_type == e_drawtarget_type_allegro) {
		auto &draw_allegro = dynamic_cast<c_drawtarget_allegro &>( drawtarget ); // get the drawtarget as ALLEGRO surface

		for (auto &layer : draw_allegro.m_layer) { // get each layer
			for (auto &obj : m_objects) { // draw elements to this layer
				obj->draw_allegro(draw_allegro, *layer);
			}
			for (auto &cable : m_cable_direct) {
				cable.draw_allegro(draw_allegro, *layer);
			}
		}
	} // ALLEGRO implementation
	
	else if (drawtarget.m_drawtarget_type == e_drawtarget_type_opengl) {
        auto &draw_opengl = dynamic_cast<c_drawtarget &>( drawtarget );
		for (auto &layer : draw_opengl.m_layer) { // get each layer
			for (auto &obj : m_objects) { // draw elements to this layer
                obj->draw_opengl(draw_opengl, *layer);
			}
		}
	}
}

void c_world::connect_nodes (c_object &first, c_object &second) {
	try {
		c_osi2_switch &node_a = dynamic_cast<c_osi2_switch &>(first);
		c_osi2_switch &node_b = dynamic_cast<c_osi2_switch &>(second);
		node_a.connect_with(node_b.use_nic(node_b.get_last_nic_index() + 1), *this);
	}
	catch(std::bad_cast) { _erro("Can not use the two objects together as network objects!"); }
}


void c_world::load (const string &filename) {
	// @TODO broken untill rewrite for net2
	_warn("LOAD code is now DISABLED in this version");

/*	
	_note("start load nodes from " << filename);
	ifstream input_file(filename);

	const int line_max_size = 8192;
	char line[line_max_size];

	string str_line;
	t_pos x, y;
	t_cjdaddr address;
	string type, name;
	while (input_file.getline(line, line_max_size)) {
		str_line = move(string(line));
		//std::cout << str_line << std::endl;
		if (str_line.find('{') != std::string::npos && str_line.find("connections") == std::string::npos) { // TODO sections
			str_line.erase(0, 4); // 4 characters
			str_line.erase(str_line.end() - 2); // TODO
			str_line.erase(str_line.end() - 1); // TODO
			type = std::move(str_line);
		} else if (str_line.find('x') != std::string::npos) {
			x = stol(str_line.substr(2));
		} else if (str_line.find('y') != std::string::npos) {
			y = stol(str_line.substr(2));
		} else if (str_line.find("name") != std::string::npos) {
			name = std::move(str_line.substr(5));
		} else if (str_line.find("IP") != std::string::npos) {
			address = str_line.substr(3);
		} else if (str_line.find('}') != std::string::npos) { // save to vector
            if (type == "cjddev")
                m_objects.push_back(make_shared<c_cjddev>(name, x, y, address));
            else if (type == "tnetdev")
                m_objects.push_back(make_shared<c_tnetdev>(name, x, y, address));
			else if (type == "userdev")
				m_objects.push_back(make_shared<c_userdev>(name, x, y, address));
		} else if (str_line.find("connections") != std::string::npos) {
			while (input_file.getline(line, line_max_size) || str_line.find('}') != std::string::npos) {
				str_line = move(string(line));
				if (str_line.find("=>") != std::string::npos) {
					istringstream ss;
					ss.str(str_line);
					t_cjdaddr ip_1, ip_2;
					ss >> ip_1;
					string tmp;
					ss >> tmp; // "=>"
					ss >> ip_2;
					unsigned int price;
					ss >> price;
					shared_ptr<c_cjddev> node_1;
					shared_ptr<c_cjddev> node_2;
					for (auto node : m_objects) {
						shared_ptr<c_cjddev> node_ptr = std::dynamic_pointer_cast<c_cjddev>(node);
						if (node_ptr->get_address() == ip_1) {
							node_1 = node_ptr;
						} else if (node_ptr->get_address() == ip_2) {
							node_2 = node_ptr;
						}
						if (node_1 && node_2) {
							node_1->add_neighbor(node_2, price);
							node_1.reset();
                            node_2.reset();
						}
					}
				}
			}
		}
	}
	for (auto &object: m_objects) {
		try {
	// @TODO remove deprecated?
	//		m_network->add_node(std::dynamic_pointer_cast<c_cjddev>(object));
		}
		catch(std::bad_cast& bc) {
			_note("exception: " << bc.what());
		}
		catch(...) {
			_note("exception");
		}
	}
	
	for (auto &object: m_objects) {
		std::dynamic_pointer_cast<c_cjddev>(object)->start_dht();
	}
	*/
	
	_note("end of load");
}

void c_world::serialize(ostream &stream) const {
	// @TODO broken untill rewrite for net2
	_warn("serialize code is now DISABLED in this version");
	
	/*
	for (auto object : world.m_objects) {
		if (typeid(*object) == typeid(c_cjddev)) {
			stream << "add cjddev {" << std::endl;
		} else if (typeid(*object) == typeid(c_tnetdev)) {
			stream << "add tnetdev {" << std::endl;
		} else if (typeid(*object) == typeid(c_userdev)) {
			stream << "add userdev {" << std::endl;
		} else
			return stream;

		shared_ptr<c_cjddev> cjddev_ptr = std::dynamic_pointer_cast<c_cjddev>(object);
		stream << "x " << cjddev_ptr->m_x << std::endl;
		stream << "y " << cjddev_ptr->m_y << std::endl;
		stream << "name " << object->get_name() << std::endl;
		stream << "IP " << cjddev_ptr->get_address() << std::endl;
		stream << "}" << std::endl;
	}

	stream << "connections {" << std::endl;

	for (auto object : world.m_objects) {
		shared_ptr<c_cjddev> cjddev_ptr = std::dynamic_pointer_cast<c_cjddev>(object);
		for (auto neighbor_address : cjddev_ptr->get_neighbors_addresses()) {
			stream << cjddev_ptr->get_address() << " => " << neighbor_address << " " <<
			cjddev_ptr->get_price(neighbor_address) << std::endl;
		}
	}

	stream << "}" << std::endl;
	*/
}
