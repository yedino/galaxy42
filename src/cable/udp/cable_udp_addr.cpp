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

std::string c_cable_udp_addr::cable_type_name() const {
	return "udp"s;
}

bool c_cable_udp_addr::is_same(const c_cable_base_addr &other) const{
	try {
		return any_cast<t_addr>(other.get_addrdata())  ==  any_cast<t_addr>(get_addrdata());
	} catch(...) {
		_erro("in catch");
		return 0; // the other address has even other type then me, so it's different
	}
}

int c_cable_udp_addr::compare(const c_cable_base_addr &other) const{
	try {
		const t_addr & my_addr    = any_cast<t_addr>(this->get_addrdata());
		const t_addr & other_addr = any_cast<t_addr>(other.get_addrdata());
		if (my_addr < other_addr) return -1;
		if (other_addr < my_addr) return +1;
		_check(my_addr == other_addr);
		return 0; // same
	} catch(...) {
		return this->cable_type_name().compare( other.cable_type_name() );
	}
}
