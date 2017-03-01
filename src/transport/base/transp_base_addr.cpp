
#include "libs1.hpp"
#include "transport/base/transp_base_addr.hpp"

const boost::any & c_transport_base_addr::get_addrdata() const { return m_addrdata ; }

c_transport_base_addr::c_transport_base_addr(boost::any && addrdata)
	: m_addrdata(std::move(addrdata))
{ }

void c_transport_base_addr::init_addrdata(boost::any && addrdata) {
	swap(m_addrdata, addrdata);
}

std::ostream & operator<<(std::ostream & ostr , c_transport_base_addr & obj) {
	obj.print(ostr);
	return ostr;
}

void c_transport_base_addr::print(std::ostream & ostr) const {
	_UNUSED(ostr);
	_throw_error_runtime("Trying to print abstract transport address");
	// ostr << "[unknown-address-type]";
}


