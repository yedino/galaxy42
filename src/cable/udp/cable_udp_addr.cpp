#include "cable_udp_addr.hpp"
#include "../../galaxy42_utils.hpp"

using namespace boost::asio::ip;

c_cable_udp_addr::c_cable_udp_addr(const t_addr &endpoint)
	: c_cable_base_addr(t_cable_kind::kind_udp), m_addr(endpoint)
{
}

c_cable_udp_addr::c_cable_udp_addr(const std::string &ip_string)
	: c_cable_base_addr(t_cable_kind::kind_udp) // filled in below
{
	std::pair<std::string, int> parsed = tunserver_utils::parse_ip_string(ip_string);
	udp::endpoint endpoint(address::from_string(parsed.first), static_cast<unsigned short>(parsed.second));
	m_addr = endpoint;
}

unique_ptr<c_cable_base_addr> c_cable_udp_addr::clone() const { ///< polymorphic clone
	unique_ptr<c_cable_udp_addr> ret = make_unique<c_cable_udp_addr>( m_addr );
	unique_ptr<c_cable_base_addr> ret_base = std::move( ret );
	return ret_base;
}

signed char c_cable_udp_addr::compare_same_class(const c_cable_base_addr & other) const {
	try {
		auto other_obj = dynamic_cast<const c_cable_udp_addr&>( other );
		if ((this->m_addr) < (other_obj.m_addr)) return -1;
		if ((this->m_addr) > (other_obj.m_addr)) return +1;
		return 0;
	} catch(const std::bad_cast &) { _throw_error_runtime("Can not compare addresses, it is other class"); }
}

void c_cable_udp_addr::print(std::ostream &ostr) const {
	ostr << m_addr;
}

