#include "transport_udp_addr.hpp"

c_transport_udp_addr::c_transport_udp_addr(const boost::asio::ip::udp::endpoint &endpoint)
:
	m_endpoint(endpoint)
{
}

void c_transport_udp_addr::print(std::ostream &ostr) const {
	  ostr << m_endpoint;
}

const boost::asio::ip::udp::endpoint &c_transport_udp_addr::get_native() {
	return m_endpoint;
}
