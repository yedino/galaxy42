#include"c_dijkstry.hpp"

c_dijkstry01::c_dijkstry01(c_osi2_switch &A,c_osi2_switch &B) :
	m_route_ok(false),
	m_start(A),
	m_target(B)
{
	find_route();
	print_all();
	print_uuid_route();
}

c_dijkstry01::c_dijkstry01(c_osi2_switch &A,c_osi2_switch &B, c_world &world) :
	m_route_ok(false),
	m_start(A),
	m_target(B)
{
	find_route();
	print_name_route(world);
	print_uuid_route();
}

void c_dijkstry01::find_route() {
	m_nodes_routed.insert(getID(m_start));
	if(getID(m_start) == getID(m_target)) {
		std::cout << "No route needed! : " << "start_point == end_point" << std::endl;
		return;
	}
	empl_nics_toMap(m_start,0);

	for(m_map_it = m_map_ofRoute.begin();
		m_map_it != m_map_ofRoute.end() && !m_route_ok; ++m_map_it) {
		c_osi2_switch &next_switch = (m_map_it->second.second).get_my_switch();
		empl_nics_toMap(next_switch,m_map_it->first);
		m_nodes_routed.insert(getID(next_switch));
	}
	calc_route_as_uuidList();

	std::cout << "targetID : " << getID(m_target) << std::endl;
}

void c_dijkstry01::empl_nics_toMap(c_osi2_switch &sw, t_osi2_cost cost_to) {
	for(size_t i = 0; i <= sw.get_last_nic_index(); ++i) {
		c_osi2_nic &nic = sw.get_nic(i);
		t_osi2_cost cost;
		c_osi2_nic * nic2 = nic.get_connected_card_or_null(cost);
		if(!nic2) {
			continue;
		}
		std::pair<c_osi2_nic&,c_osi2_nic&>local_pair(nic,*nic2);
		if(std::find(m_nodes_routed.begin(),m_nodes_routed.end(),getID(nic2->get_my_switch())) == m_nodes_routed.end()) {

			//draw_packet(nic.get_my_switch(), nic.get_my_switch());

			m_map_ofRoute.emplace(cost+cost_to,local_pair);
			m_nodes_routed.insert(getID(nic2->get_my_switch()));
			//std::cout << "adding2 " << getID(nic2->get_my_switch()) << std::endl; // dbg
		}
		if (getID(*nic2) == getID(m_target)) {
			std::cout << "Found route!" << std::endl;
			m_route_ok = true;
			break;
		}
	}
}

void c_dijkstry01::draw_packet(c_osi2_switch& source, c_osi2_switch& target) {
t_osi3_packet pckg {"loking for target!",
					target.get_uuid_any(),
					source.get_uuid_any()};
source.send_package(std::move(pckg));
}

void c_dijkstry01::print_all() {
	std::cout << "check all map:" << std::endl;
	for(auto &ptr_object : m_map_ofRoute) {
        _dbg1("DEBUG1: " << ptr_object.second.first);
		std::cout << "cost: " << ptr_object.first
			  << " for switch: " << getID(ptr_object.second.first) << " -> " << getID(ptr_object.second.second)
			  << "\n\tfor nic: " << ptr_object.second.first << " -> " << ptr_object.second.second << std::endl;
	}
}

void c_dijkstry01::calc_route_as_uuidList() {
	if(!m_map_ofRoute.empty()) {
		m_map_it = --m_map_ofRoute.end();
		m_last_routeList.emplace_front(getID(m_map_it->second.second));
		t_osi3_uuid searched = getID(m_map_it->second.first);
		while(m_map_it != m_map_ofRoute.begin()) {
				if(searched == getID(m_map_it->second.second)) {
					m_last_routeList.emplace_front(getID(m_map_it->second.second));
					searched = getID(m_map_it->second.first);
				}
				m_map_it--;
			}
			if(searched == getID(m_map_it->second.first)) {
				m_last_routeList.emplace_front(searched);
			} else {
				m_last_routeList.emplace_front(searched);
				m_last_routeList.emplace_front(getID(m_map_it->second.first));
			}
	} else {
		m_last_routeList.emplace_front(getID(m_start));
	}
}

std::list<t_osi3_uuid>& c_dijkstry01::get_last_routeList() {
	return m_last_routeList;
}

t_osi3_uuid c_dijkstry01::get_next_uuid() {
	std::list<t_osi3_uuid>::iterator it_ls = m_last_routeList.begin();
	it_ls++;
	return *it_ls;
}

c_osi2_nic& c_dijkstry01::get_next_nic() {
	if(m_last_routeList.size() < 2) {
		std::cout << "There are no next nic: target has been reached!" << std::endl;
		return m_map_ofRoute.begin()->second.first;
	} else {
		for(auto &obj : m_map_ofRoute) {
			if(obj.second.second.get_my_switch().get_uuid_any() == get_next_uuid()) { // *(++(... get second list element
				return obj.second.first;
			}
		}
		std::cout << "Something went wrong: return source nic" << std::endl;
		return m_map_ofRoute.begin()->second.first;
	}
}

void c_dijkstry01::print_uuid_route() {
	std::cout << "route by switch uuid: " << std::endl;
	for(auto &id : m_last_routeList) {
		std::cout << id << " --> ";
	}
	std::cout << "the_end" << std::endl;
}

void c_dijkstry01::print_name_route(c_world &world) {
	std::cout << "route by switch name: " << std::endl;
	for(auto &id : m_last_routeList) {
		std::cout << world.find_object_by_uuid_as_object(id).get_name() << " --> ";
	}
	std::cout << "the_end" << std::endl;
}

t_osi3_uuid c_dijkstry01::getID(c_osi2_nic &nic) {
	return nic.get_my_switch().get_uuid_any();
}

t_osi3_uuid c_dijkstry01::getID(c_osi2_switch &sw) {
	return sw.get_uuid_any();
}
