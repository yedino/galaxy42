#include "cable/simulation/cable_simul_addr.hpp"


c_cable_simul_addr::c_cable_simul_addr(int uuid) : c_cable_base_addr(t_cable_kind::kind_simul),
m_addr(uuid)
{ }

unique_ptr<c_cable_base_addr> c_cable_simul_addr::clone() const { ///< polymorphic clone
	return make_unique< c_cable_simul_addr >( m_addr );
}

void c_cable_simul_addr::print(ostream & ostr) const {
	auto uuid = m_addr;
	ostr << "[transp-simul-" << uuid << "]" ;
}

signed char c_cable_simul_addr::compare_same_class(const c_cable_base_addr & other) const {
	try {
		auto other_obj = dynamic_cast<const c_cable_simul_addr&>( other );
		if ((this->m_addr) < (other_obj.m_addr)) return -1;
		if ((this->m_addr) > (other_obj.m_addr)) return +1;
		return 0;
	} catch(std::bad_cast) { _throw_error_runtime("Can not compare addresses, it is other class"); }
}

