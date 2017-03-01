
#include "transport/base/transp_base_obj.hpp"

#include "libs1.hpp"


c_transport_base_obj::c_transport_base_obj() {
	_dbg1("Creating some transport");
}

void c_transport_base_obj::send_data(boost::any dest, const unsigned char *data, size_t size_of_data) {
	_UNUSED(dest);
	_UNUSED(data);
	_UNUSED(size_of_data);
	_throw_error_runtime("Abstract transport used - unsupported transport type?");
}

