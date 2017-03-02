#include "cable/simulation/cable_simul_addr.hpp"


c_cable_simul_addr::c_cable_simul_addr(int uuid) : c_cable_base_addr(uuid) { }

void c_cable_simul_addr::print(ostream & ostr) const {
	auto uuid = boost::any_cast<int>(get_addrdata());
	ostr << "[transp-simul-" << uuid << "]" ;
}

