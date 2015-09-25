#include "c_file_loader.hpp"
#include <fstream>

using namespace std;

c_file_loader::c_file_loader(c_world  *world):m_world(world)
{

}

void c_file_loader::load(string p_filename){

	std::string str_line;


	const int line_max_size = 8192;
	char line[line_max_size];
	std::vector<std::string> tmp_object;

	ifstream input_file(p_filename);
	while (input_file.getline(line,line_max_size)){
		str_line = move(string(line));
		if(str_line.find('{') != string::npos) {
			tmp_object.clear();
			tmp_object.push_back(str_line);
		}else if(str_line.find('}')!= string::npos){
			tmp_object.push_back(str_line);
			read_cobject(tmp_object);
		}else{
			tmp_object.push_back(str_line);
		}
	}

}



void c_file_loader::read_cobject(std::vector <std::string > p_input) {

//	c_object tmp_object;
	int tmp_x;
	int tmp_y;
	std::string tmp_name;

	for(auto m_line:p_input){
		if(m_line.find('x') != string::npos){
			tmp_x = stol(m_line.substr(2));
//			tmp_object->set_x(tmp_int);
		}else if(m_line.find('y')!= string::npos){
			tmp_y = stol(m_line.substr(2));

//			tmp_object->set_y(tmp_int);
		}else if(m_line.find("name")!=string::npos){
			try{
				tmp_name = m_line.substr(5);
			}catch(...){
				std::cout<< "error line"<<m_line<<std::endl;
			}
//			tmp_obj->set_name(tmp_string);
//		} else if (m_line.find("IP") != std::string::npos) {
//			tmp_string = m_line.substr(3);
////			tmp_obj->set_IP(tmp_string);
		}else{
			//unrecognized
		}
	}


	if(p_input.at(0).find("connections") != string::npos){
		//reading connections;
		read_connection(p_input);
	}else if(p_input.at(0).find("switch")!= string::npos){
//		tmp_obj = tworzenie nowego switcha i podlaczanie go do swiata
			m_world->m_objects.push_back(unique_ptr <c_osi2_switch> (new c_osi2_switch(*m_world,tmp_name,tmp_x,tmp_y)));
	}else if(p_input.at(0).find("node") !=  string::npos){
			m_world->m_objects.push_back(unique_ptr <c_node> (new c_node(*m_world,tmp_name,tmp_x,tmp_y)));
	}else{
		//something goes wrong
	}
}


void c_file_loader::read_connection(std::vector<std::string>p_input) {

	for (auto str_line:p_input){

		if (str_line.find("=>") == std::string::npos) {
			continue;
		}

		stringstream ss;
		ss.str(str_line);
//		long int ip_1, ip_2;
		string name1,name2;
		ss >> name1;
		string tmp;
		ss >> tmp; // "=>"
		ss >> name2;
		unsigned int price;
		ss >> price;
//		c_osi2_nic* nic1;
//		c_osi2_nic* nic_2;
		unique_ptr<c_object> first = nullptr;
		unique_ptr<c_object> second = nullptr;

		bool found_first =false;
		for(auto & node:m_world->m_objects){
			if(node->get_name() == name1 || node->get_name() == name2 ){
				if(!found_first){
					first =std::move(node);
					found_first = true;
				}else{
					second =std::move( node);
					break;
				}
			}
		}

		if(first == nullptr || second == nullptr){
			break;
		}

		m_world->connect_nodes(first,second);


//		bool m_found_first = false;
//		bool do_break= false;

		//pytanie po czym laczymy  - adresy kart sieciowych??
//		istringstream ss;
//		ss.str(str_line);
//		t_cjdaddr ip_1, ip_2;
//		ss >> ip_1;
//		string tmp;
//		ss >> tmp; // "=>"
//		ss >> ip_2;
//		unsigned int price;
//		ss >> price;
//		shared_ptr<c_cjddev> node_1;
//		shared_ptr<c_cjddev> node_2;
//		for (auto node : m_objects) {
//			shared_ptr<c_cjddev> node_ptr = std::dynamic_pointer_cast<c_cjddev>(node);
//			if (node_ptr->get_address() == ip_1) {
//				node_1 = node_ptr;
//			} else if (node_ptr->get_address() == ip_2) {
//				node_2 = node_ptr;
//			}
//			if (node_1 && node_2) {
//				node_1->add_neighbor(node_2, price);
//				node_1.reset();
//				node_2.reset();
//			}
	}
}

void c_file_loader::save(string p_filename)
{
	std::ofstream stream(p_filename);
	for (auto & object : m_world->m_objects) {

		if (typeid(*object) == typeid(c_cjddev)) {
			stream << "add cjddev {" << std::endl;
		} else if (typeid(*object) == typeid(c_node)) {
			stream << "add node {" << std::endl;
		} else if (typeid(*object) == typeid(c_osi2_switch)) {
			stream << "add switch {" << std::endl;
		} else{

		}
		//	return stream;

//		unique_ptr_ptr<c_cjddev> cjddev_ptr = std::dynamic_pointer_cast<c_cjddev>(object);
		stream << "x " << object->get_x() << std::endl;
		stream << "y " << object->get_y() << std::endl;
		stream << "name " << object->get_name() << std::endl;
		stream << "}" << std::endl;
	}


	stream << "connections {" << std::endl;

	for (auto & object : m_world->m_cable_direct) {
//		stream<<object->endpoint;
		stream<<object.get_endpoints().at(0).get().get_serial_number();
		stream<<"=>"<<std::endl;
		stream<<object.get_endpoints().at(1).get().get_serial_number();

//		shared_ptr<c_> cjddev_ptr = std::dynamic_pointer_cast<c_cjddev>(object);
//		for (auto neighbor_address : cjddev_ptr->get_neighbors_addresses()) {
//			stream << cjddev_ptr->get_address() << " => " << neighbor_address << " " <<
//			cjddev_ptr->get_price(neighbor_address) << std::endl;
//		}

	}

	stream<<"}"<<std::endl;


}
