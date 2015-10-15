#ifndef C_DIJKSTRY_HPP
#define C_DIJKSTRY_HPP

#include "libs1.hpp"
#include "osi2.hpp"
#include "c_node.hpp"
#include "c_world.hpp"
//#include <algorithm>

class c_dijkstry01 {
  public:
	c_dijkstry01(c_osi2_switch &A,c_osi2_switch &B) :
			m_route_ok(false),
			m_start(A),
			m_target(B) {
		find_route();
		print_all();
		print_uuid_route();
	}
	c_dijkstry01(c_osi2_switch &A,c_osi2_switch &B, c_world &world) :
			m_route_ok(false),
			m_start(A),
			m_target(B) {
		find_route();
		print_name_route(world);
		print_uuid_route();
	}
	void find_route();
	void print_all();
	void print_uuid_route();
	void print_name_route(c_world&);
	bool m_route_ok;

  private:
	void empl_nics_toMap(c_osi2_switch &, t_osi2_cost);
	c_osi2_switch & m_start;
	c_osi2_switch & m_target;

	std::multimap<unsigned int, std::pair<c_osi2_nic&,c_osi2_nic&>> m_map_ofRoute;
	std::multimap<unsigned int, std::pair<c_osi2_nic&,c_osi2_nic&>>::iterator m_map_it;
	std::set<t_osi3_uuid> m_nodes_routed;

	t_osi3_uuid getID(c_osi2_nic& nic) { return nic.get_my_switch().get_uuid_any(); }
	t_osi3_uuid getID(c_osi2_switch& sw) { return sw.get_uuid_any(); }
};

#endif // C_DIJKSTRY_HPP
