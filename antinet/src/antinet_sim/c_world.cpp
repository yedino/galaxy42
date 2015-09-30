#include "c_world.hpp"

#include "c_drawtarget_opengl.hpp"
#include "c_file_loader.hpp"

long int c_world::s_nr = 0;


c_world::c_world()
	: m_nr( s_nr++ )
{
	
}

void c_world::add_osi2_switch(const std::string &name, int x, int y)
{
	m_objects.emplace_back( make_unique<c_osi2_switch>(*this, name,x,y)  );
}

void c_world::add_node(const std::string &name, int x, int y)
{
	m_objects.emplace_back( make_unique<c_node>(*this, name,x,y)  );
}

c_osi2_cable_direct & c_world::new_cable_between(c_osi2_nic &a, c_osi2_nic &b, t_osi2_cost cost)
{
	m_cable_direct.emplace_back( make_unique<c_osi2_cable_direct>( a, b, cost ));
	return * m_cable_direct.back();
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
	


	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_1", 200, 200));
	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_2", 250, 100));
	m_objects.emplace_back( make_unique<c_osi2_switch>(*this, "SWITCH_1", 400, 150));
	
	connect_network_devices(
	      	unique_cast_ref<c_object &>(m_objects.at(1)), 
	      	unique_cast_ref<c_object &>(m_objects.at(2)),
	      	1
	      );

//	load("layout/current/map2.txt");

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
			
//			for (auto &cable : m_cable_direct) {
//				cable.draw_allegro(draw_allegro, *layer); // TODO add some drawing for the cable
//			}
			
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

void c_world::connect_network_devices (c_object &first, c_object &second, t_osi2_cost cost) {
	try {
		c_osi2_switch &node_a = dynamic_cast<c_osi2_switch &>(first);
		c_osi2_switch &node_b = dynamic_cast<c_osi2_switch &>(second);
		node_a.connect_with(node_b.use_nic(node_b.get_last_nic_index() + 1), *this, cost);
	}
	catch(std::bad_cast) { _erro("Can not use the two objects together as network objects!"); }
}

void c_world::connect_network_devices(size_t nr_a, size_t nr_b, t_osi2_cost cost) {
	connect_network_devices( * m_objects.at(nr_a) , * m_objects.at(nr_b) , cost);
}

void c_world::connect_network_devices(const std::string &nr_a, const std::string &nr_b, t_osi2_cost cost)
{
	connect_network_devices( find_object_by_name_as_index(nr_a) , find_object_by_name_as_index(nr_b) , cost);
}

void c_world::print_route_between(c_object &first, c_object &second) {
	_mark("WORLD ROUTE");
	c_osi2_switch & swA = dynamic_cast<c_osi2_switch&>(first);
	c_osi2_switch & swB = dynamic_cast<c_osi2_switch&>(second);
	
	_note("swA:\n\n"<<swA);
	_note("swB:\n\n"<<swB);
	
	// of course all network objects, 
	// like this->m_objects, this->m_cable_direct etc must remain valid (no remove/add/realloc/etc)
	// therefore we can operate on references, or simple pointers to them here
	
	std::multimap<t_osi2_cost, c_osi2_switch*> visits; // priority queue of things to visit
	typedef std::pair<t_osi2_cost, c_osi2_switch*> t_visit; 
	
	const t_osi2_cost cost_infinite = 999999; // TODO std limits
	
	struct dijkstra_info {
		t_osi2_cost m_cost;
		c_osi2_switch *m_parent;
		
		dijkstra_info() : m_cost(cost_infinite), m_parent(nullptr) { }
	};
	
	std::map< c_osi2_switch* , dijkstra_info > cost_of_sw; // map to add property of current-dijkstra-cost to the
	// discovered nodes
	
	_info("Adding the starting point:");
	cost_of_sw[ & swA ].m_cost = 0;
	visits.insert(t_visit(0, & swA));
	
	int cycle=0; // dbg
	while (true) {
		_info("\n\ncycle="<<cycle);
		++cycle;	if (cycle>20) break; // dbg
		
		_info("---visits---");
		for (auto & pair : visits) {
			_info("At COST="<<pair.first<<" we have: "<<pair.second<<" "<<pair.second->print_str(-2));
		}
		_info("---visits---");
		
		_info("---costs---");
		for (auto & pair : cost_of_sw) {
			const c_osi2_switch & sw = * pair.first;
			_info("For sw="<<pair.first<<" that is "<<sw.print_str(-2)
				<<"we have: cost="<<pair.second.m_cost
				<<"parent="<<pair.second.m_parent
				<< (
			     pair.second.m_parent == nullptr ? 
			     string(" (noparent)")
			     :
			     string(" that is: ") + pair.second.m_parent->print_str(-2)
			     )
			);
		}
		_info("---costs---");
	
		{
			if (visits.begin() == visits.end()) break; // <--- all
			
			auto best_as_iterator = visits.begin(); // currently best candidate is at begin
			
			const t_osi2_cost & best_cost = visits.begin()->first;
			c_osi2_switch & best_sw       = * visits.begin()->second;
			_info("Discovering, from best_sw:"<<best_sw.print_str(-2)<<" at cost best_cost="<<best_cost);
			_info("Best_sw is:" << best_sw);
			for (size_t ix=0; ix<=best_sw.get_last_nic_index(); ++ix) {
				c_osi2_nic & child_nic = best_sw.get_nic(ix); // child nic
				
				t_osi2_cost child_cost;
				c_osi2_nic * child_nic_B = child_nic.get_connected_card_or_null(child_cost);
				
				if (child_nic_B) { // if there is a NIC card connected at end of this NIC
					c_osi2_switch & child_sw_B = child_nic_B->get_my_switch();
					_info("At ix="<<ix<<" we have connected other side: "
					      <<" at cost="<<child_cost
					      <<" switch: "<<child_sw_B.print_str(-2)
					      <<" via it's card NIC_B="<<child_nic_B);

					t_osi2_cost child_cost_full = child_cost + best_cost;
					
					if (cost_of_sw[ & child_sw_B	 ].m_cost <= child_cost_full) {
						_info("But we already had as good or better route, NOT ADDING");
					}
					else {
						cost_of_sw[ & child_sw_B ].m_cost = child_cost_full;
						cost_of_sw[ & child_sw_B ].m_parent = & best_sw; // the current best sw is the parent
						_note("Adding this child to visits as: COST=" << child_cost_full << " " << child_sw_B.print_str(-2));
						visits.insert(t_visit(child_cost_full, & child_sw_B));
					}
				} // connected end
				
			}	 // all children
			
			_note("Removing the current best");
			visits.erase( best_as_iterator ); // we're done discovering this one
			// !!! --- warning this invalidated any other iterators now!
		} // the best switch currently
		
	} // entire algorithm main loop
	
	_info("Algorithm is done.");
	// print all hops that we need to take:
	c_osi2_switch * hop_ptr = cost_of_sw[ & swB ].m_parent;
	
	while (hop_ptr != nullptr) {
		_info("We go through " << hop_ptr << " that is: " << hop_ptr->print_str(-2));
		hop_ptr = cost_of_sw[ hop_ptr ].m_parent;
	}
	_info("That is all.");
	
	// print the result
}

size_t c_world::find_object_by_name_as_index(const std::string &name) const {
	for (size_t ix=0; ix<m_objects.size(); ++ix) if (m_objects[ix]->get_name() == name) return ix;
	throw std::out_of_range( string("Can not find object with name=") + name);
}

c_object &c_world::find_object_by_name_as_object(const std::string &name)
{
	for (size_t ix=0; ix<m_objects.size(); ++ix) if (m_objects[ix]->get_name() == name) return * m_objects[ix];
	throw std::out_of_range( string("Can not find object with name=") + name);
}

c_osi2_switch & c_world::find_object_by_name_as_switch(const std::string &name)
{
	const string name_of_type("c_osi2_switch"); // match our return type!
	try {
		return dynamic_cast<c_osi2_switch &>( find_object_by_name_as_object(name) );
	} catch(...) { }
	
	throw std::out_of_range(
		string("Can not find object (of type ")
		+ name_of_type + string(") with name=") + name);
}


void c_world::load (const string &filename) {
	// @TODO broken untill rewrite for net2
//	_warn("LOAD code is now DISABLED in this version");

	c_file_loader loader(this);
	loader.load(filename);

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

void c_world::serialize(ostream &stream) {
	// @TODO broken untill rewrite for net2
//	_warn("serialize code is now DISABLED in this version");
	c_file_loader(this).save(stream);


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
