#include "c_world.hpp"

#include "c_drawtarget_opengl.hpp"
#include "c_file_loader.hpp"
#include "c_dijkstry.hpp"

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
	

/*
	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_1", 200, 200));
	m_objects.emplace_back( make_unique<c_node>(*this, "NODE_2", 250, 100));
	m_objects.emplace_back( make_unique<c_osi2_switch>(*this, "SWITCH_1", 400, 150));
	
	connect_network_devices(
	      	unique_cast_ref<c_object &>(m_objects.at(1)), 
	      	unique_cast_ref<c_object &>(m_objects.at(2)),
	      	1
	      );
*/

//	load("layout/current/map2.txt");

	add_node("nodeA",100,100); // ***
	add_node("nodeB",300,100);
	add_node("nodeC1",300,300);
	add_node("nodeC2",300,320);
	add_node("nodeC3",300,340);
	add_node("nodeC4",300,360);
	add_node("nodeD",150,200);
	add_node("nodeE",400,100); // ***
	
	add_osi2_switch("swA", 200,100);
	add_osi2_switch("swB", 300,100);
	add_osi2_switch("swC", 200,300);
	add_osi2_switch("swD", 300,100);
	
	_mark("Connecting devices");
	connect_network_devices("nodeA","swA", 1);
	connect_network_devices("swA","swB", 1);
	connect_network_devices("swB","swD", 1);
	connect_network_devices("nodeA","nodeD", 1);
	connect_network_devices("swA","swC", 1);
	connect_network_devices("swC","nodeC1", 1);
	connect_network_devices("swC","nodeC2", 1);
	connect_network_devices("swC","nodeC3", 1);
	connect_network_devices("swC","nodeC4", 1);
	connect_network_devices("swD","nodeE", 1); // ***

}


void c_world::tick () {
	static unsigned int tick_number = 0;
	m_simclock += get_chronon(); // @TODO move the 0.1 speed to a (const?) variable
	
	for (auto &obj : m_objects) {
		try {
			obj->logic_tick();
		}
		catch(...) {
			_erro("Error in logic ticks");
			throw ;
		}
	}
	
	for (auto &obj : m_objects) {
		try {
			obj->recv_tick();
		}
		catch(...) {
			_erro("Error in recv ticks ticks");
			throw ;
		}
	}
	
	for (auto &obj : m_objects) {
		try {
			obj->send_tick();
		}
		catch(...) {
			_erro("Error in recv send ticks");
			throw ;
		}
	}
	
	std::cout << "****************END OF TICK (" << tick_number << ")****************" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1)); // XXX
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

size_t_maybe c_world::route_next_hop_nic_ix(c_object &first, c_object &second)
{
	/* @TODO delete
	 * c_osi2_switch * next_switch = this->route_find_route_between_or_null(first,second);
	if (next_switch == nullptr) return size_t_invalid();
	return dynamic_cast<c_osi2_switch&>(first).find_which_nic_goes_to_switch_or_invalid(next_switch);
	*/
	//_dbg1("route_next_hop_nic_ix");


	t_osi2_route_result route = route_find_route_between(first, second);
	if (!route.valid) return size_t_invalid(); /// error
	_dbg1("next hop nic index: " << route.first_hop_nic_ix);
	return route.first_hop_nic_ix;
}

t_osi2_route_result c_world::route_find_route_between(c_object &first, c_object &second) {
	t_osi2_route_result result;
	result.valid=false;
	
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
	
//	_info("Adding the starting point:");
	cost_of_sw[ & swA ].m_cost = 0;
	visits.insert(t_visit(0, & swA));
	
	int cycle=0; // dbg
	while (true) {
//		_info("\n\ncycle="<<cycle);
		++cycle;	//if (cycle>20) break; // dbg
		
//		_info("---visits---");
		for (auto & pair : visits) {
//			_info("At COST="<<pair.first<<" we have: "<<pair.second<<" "<<pair.second->print_str(-2));
		}
//		_info("---visits---");
		
//		_info("---costs---");
/*		for (auto & pair : cost_of_sw) {
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
*/
		{
			if (visits.begin() == visits.end()) break; // <--- all
			
			auto best_as_iterator = visits.begin(); // currently best candidate is at begin
			
			const t_osi2_cost & best_cost = visits.begin()->first;
			c_osi2_switch & best_sw       = * visits.begin()->second;
//			_info("Discovering, from best_sw:"<<best_sw.print_str(-2)<<" at cost best_cost="<<best_cost);
//			_info("Best_sw is:" << best_sw);
			for (size_t ix=0; ix<=best_sw.get_last_nic_index(); ++ix) {
				c_osi2_nic & child_nic = best_sw.get_nic(ix); // child nic
				
				t_osi2_cost child_cost;
				c_osi2_nic * child_nic_B = child_nic.get_connected_card_or_null(child_cost);
				
				if (child_nic_B) { // if there is a NIC card connected at end of this NIC
					c_osi2_switch & child_sw_B = child_nic_B->get_my_switch();
/*					_info("At ix="<<ix<<" we have connected other side: "
					      <<" at cost="<<child_cost
					      <<" switch: "<<child_sw_B.print_str(-2)
					      <<" via it's card NIC_B="<<child_nic_B);
*/
					t_osi2_cost child_cost_full = child_cost + best_cost;
					
					if (cost_of_sw[ & child_sw_B	 ].m_cost <= child_cost_full) {
//						_info("But we already had as good or better route, NOT ADDING");
					}
					else {
						cost_of_sw[ & child_sw_B ].m_cost = child_cost_full;
						cost_of_sw[ & child_sw_B ].m_parent = & best_sw; // the current best sw is the parent
//						_note("Adding this child to visits as: COST=" << child_cost_full << " " << child_sw_B.print_str(-2));
						visits.insert(t_visit(child_cost_full, & child_sw_B));
					}
				} // connected end
				
			}	 // all children
			
//			_note("Removing the current best");
			visits.erase( best_as_iterator ); // we're done discovering this one
			// !!! --- warning this invalidated any other iterators now!
		} // the best switch currently
		
	} // entire algorithm main loop
	
//	_info("Algorithm is done.");
	// print all hops that we need to take:
	c_osi2_switch * hop_ptr = cost_of_sw[ & swB ].m_parent;
	c_osi2_switch * next_hop = nullptr; // to return
	
	
	// calculated info about the finall route:
	t_osi2_cost cost=0; 
	result.first_hop_nic_ix = size_t_invalid();
	result.target_nic_ix = size_t_invalid();
	bool first_iteration=true;
	while (hop_ptr != nullptr) {
		_info("ROUTE IS: " << *hop_ptr);
		
		if (first_iteration) {
			result.target_nic_ix = hop_ptr->find_which_nic_goes_to_switch_or_invalid( & swB );
		}
		
		auto our_parent = cost_of_sw[ hop_ptr ].m_parent; // this is another parent to go to
		if (our_parent != nullptr) { // first node on path after starting node
			size_t nic_ix = our_parent->find_which_nic_goes_to_switch_or_invalid(hop_ptr);
			_info("nic_ix="<<nic_ix);
//			result.hops_uuid= // get UUID of that nic ix
			next_hop = hop_ptr;
		}
		hop_ptr = our_parent; // go to our parent
		
		first_iteration=false;
	}
	// TODO cost
	// TODO uuid
	
	result.first_hop_nic_ix = swA.find_which_nic_goes_to_switch_or_invalid(next_hop);
	_info("Found route, next hop is "<<next_hop);
	_info("result: A.ix="<<result.first_hop_nic_ix<<" B.ix="<<result.target_nic_ix);
	result.valid=true;
	
	
	// print the result
	return result;
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

c_osi2_switch &c_world::find_object_by_uuid_as_switch(const t_osi3_uuid id)
{
	const string name_of_type("c_osi2_switch"); // match our return type!
	try {
		return dynamic_cast<c_osi2_switch &>( find_object_by_uuid_as_object(id) );
	} catch(...) { }

//	throw std::out_of_range(
//		string("Can not find object (of type ")
//				+ name_of_type + string(") with uuid=")+std::num_put(id) );

}


c_object& c_world::find_object_by_uuid_as_object(const t_osi3_uuid id) {

	for (size_t ix=0; ix<m_objects.size(); ++ix){
		 c_osi2_switch & tmp_obj = unique_cast_ref<c_osi2_switch&>(m_objects.at(ix));		//check if tmp_obj isn't stolen
		for(auto & nic :tmp_obj.m_nic ){
				if (nic->get_uuid() == id) return * m_objects[ix];
			}
		}
	throw std::out_of_range( string("Can not find object with id=") + std::to_string(id));
}

size_t c_world::find_object_by_uuid_as_index(const t_osi3_uuid id) {


	for (size_t ix=0; ix<m_objects.size(); ++ix){
		 c_osi2_switch & tmp_obj = unique_cast_ref<c_osi2_switch&>(m_objects.at(ix));		//check if tmp_obj isn't stolen
		for(auto & nic :tmp_obj.m_nic ){
				if (nic->get_uuid() == id) return ix;
			}
		}
	throw std::out_of_range( string("Can not find object with id=") + std::to_string(id));
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
