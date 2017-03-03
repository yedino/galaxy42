#include "cable_udp_addr.hpp"
#include "../../galaxy42_utils.hpp"

using namespace boost::asio::ip;

c_cable_udp_addr::c_cable_udp_addr(const t_addr &endpoint)
	: c_cable_base_addr(endpoint)
{
}

c_cable_udp_addr::c_cable_udp_addr(const std::string &ip_string)
	: c_cable_base_addr( boost::any() ) // filled in below
{
	std::pair<std::string, int> endpoint = tunserver_utils::parse_ip_string(ip_string);
	init_addrdata( udp::endpoint(address::from_string(endpoint.first), static_cast<unsigned short>(endpoint.second)) );
}

void c_cable_udp_addr::print(std::ostream &ostr) const {
	auto endpoint = boost::any_cast<t_addr>(get_addrdata());
	ostr << endpoint;
}

