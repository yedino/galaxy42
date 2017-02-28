#include "transport/simulation/transp_simul_addr.hpp"


c_transport_simul_addr::c_transport_simul_addr(int uuid) : m_uuid(uuid) { }


void c_transport_simul_addr::print(ostream & ostr) const {
	ostr << "[transp-simul-" << m_uuid << "]" ;
}

