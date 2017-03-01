#include "transport_udp_addr.hpp"
#include "../../galaxy42_utils.hpp"

using namespace boost::asio;

c_transport_udp_addr::c_transport_udp_addr(const boost::asio::ip::udp::endpoint &endpoint)
:
	m_endpoint(endpoint)
{
}

c_transport_udp_addr::c_transport_udp_addr(const std::string &ip_string) {
	std::pair<std::string, int> endpoint = tunserver_utils::parse_ip_string(ip_string);
	m_endpoint = ip::udp::endpoint(ip::address::from_string(endpoint.first), static_cast<unsigned short>(endpoint.second));
}

void c_transport_udp_addr::print(std::ostream &ostr) const {
	  ostr << m_endpoint;
}

const boost::asio::ip::udp::endpoint &c_transport_udp_addr::get_native() {
	return m_endpoint;
}
