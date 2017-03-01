
#include "transport/base/transp_base_obj.hpp"

#include "libs1.hpp"


c_transport_base_obj::c_transport_base_obj() {
	_dbg1("Creating some transport");
}

void c_transport_base_obj::send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) {
	_UNUSED(dest);
	_UNUSED(data);
	_UNUSED(size);
	_throw_error_runtime("Abstract transport used - unsupported transport type?");
}

