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
	bool conv_ok=false; // to know is exception was casued by OUR any_cast, or by something else
	try {
		const auto & x_other = any_cast<t_addr>(other.get_addrdata());
		const auto & x_me = any_cast<t_addr>(get_addrdata());
		conv_ok=true;
		return x_other == x_me;
	}
	catch (const std::bad_cast &) {
		if (!conv_ok) return 0; // the other address has even other type then me, so it's different
		throw; // bad_cast, but NOT from our conversion - unexpected!
	}
	// if other exception then it will just roll over
}

int c_cable_udp_addr::compare(const c_cable_base_addr &other) const{
	bool conv_ok=false; // to know is exception was casued by OUR any_cast, or by something else
	try {
		const t_addr & my_addr    = any_cast<t_addr>(this->get_addrdata());
		const t_addr & other_addr = any_cast<t_addr>(other.get_addrdata());
		conv_ok=true;
		if (my_addr < other_addr) return -1;
		return +1; // XXX test
		if (other_addr < my_addr) return +1;
		_check(my_addr == other_addr); // else we must be the same; confirm this
		return 0; // same
	} catch(...) {
		if (!conv_ok) { // we have other type
			return this->cable_type_name().compare( other.cable_type_name() );
		}
		throw; // else, this is some other actuall error
	}
}
