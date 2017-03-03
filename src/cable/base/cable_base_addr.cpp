
#include "libs1.hpp"
#include "cable/base/cable_base_addr.hpp"

const boost::any & c_cable_base_addr::get_addrdata() const { return m_addrdata ; }

c_cable_base_addr::c_cable_base_addr(boost::any && addrdata)
	: m_addrdata(std::move(addrdata))
{ }

void c_cable_base_addr::init_addrdata(boost::any && addrdata) {
	swap(m_addrdata, addrdata);
}

void c_cable_base_addr::print(std::ostream & ostr) const {
	_UNUSED(ostr);
	_throw_error_runtime("Trying to print abstract cable address");
	// ostr << "[unknown-address-type]";
}

std::ostream & operator<<(std::ostream & ostr , c_cable_base_addr & obj) {
	obj.print(ostr);
	return ostr;
}

unique_ptr<c_cable_base_addr> cable_make_addr(const string & str) {
	// TODO@mik
	_UNUSED(str);
	unique_ptr<c_cable_base_addr> x;
	return x;
}

bool operator==(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2) {
	// TODO@mik
	_UNUSED(obj2);
	return obj1.is_same(obj2);
}

bool operator<(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2) {
	// TODO@mik
	_UNUSED(obj2);
	return obj1.compare(obj2) < 0;
}

bool c_cable_base_addr::is_same(const c_cable_base_addr &other) const {
	_UNUSED(other);
	_NOTREADY();
	// TODO@mik - replace with abstract =0, when implemented for all existing cables, please
}

int c_cable_base_addr::compare(const c_cable_base_addr &other) const {
	_UNUSED(other);
	_NOTREADY();
	// TODO@mik - replace with abstract =0, when implemented for all existing cables, please
}


