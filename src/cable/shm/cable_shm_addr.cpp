#include "cable_shm_addr.hpp"

cable_shm_addr::cable_shm_addr()
: c_cable_base_addr( t_cable_kind::kind_shm )
{ }

cable_shm_addr::cable_shm_addr(const std::string &address)
: c_cable_base_addr( t_cable_kind::kind_shm ),
m_addr(address)
{
}

void cable_shm_addr::print(std::ostream & ostr) const {
	ostr << "shm: " << m_addr;
}

unique_ptr<c_cable_base_addr> cable_shm_addr::clone() const { ///< polymorphic clone
	return make_unique<cable_shm_addr>( m_addr );
}

signed char cable_shm_addr::compare_same_class(const c_cable_base_addr & other) const {
	try {
		auto other_obj = dynamic_cast<const cable_shm_addr&>( other );
		if ((this->m_addr) < (other_obj.m_addr)) return -1;
		if ((this->m_addr) > (other_obj.m_addr)) return +1;
		return 0;
	} catch(const std::bad_cast &) { _throw_error_runtime("Can not compare addresses, it is other class"); }
}

