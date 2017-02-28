
#include "libs1.hpp"
#include "transport/base/transp_base_addr.hpp"

void c_transport_base_addr::print(std::ostream & ostr) const {
	_UNUSED(ostr);
	_throw_error_runtime("Trying to print abstract transport address");
	// ostr << "[unknown-address-type]";
}

std::ostream & operator<<(std::ostream & ostr , c_transport_base_addr & obj) {
	obj.print(ostr);
	return ostr;
}


