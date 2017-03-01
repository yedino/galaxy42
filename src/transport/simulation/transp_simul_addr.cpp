#include "transport/simulation/transp_simul_addr.hpp"


c_transport_simul_addr::c_transport_simul_addr(int uuid) : c_transport_base_addr(uuid) { }

void c_transport_simul_addr::print(ostream & ostr) const {
	auto uuid = boost::any_cast<int>(get_addrdata());
	ostr << "[transp-simul-" << uuid << "]" ;
}

