#ifdef __MACH__
#include "c_tuntap_macosx_obj.hpp"

c_tuntap_macosx_obj::c_tuntap_macosx_obj() {
	_NOTREADY_warn();
}

size_t c_tuntap_macosx_obj::send_to_tun(const unsigned char *data, size_t size) {
	_NOTREADY();
	_UNUSED(data);
	_UNUSED(size);
	return 0;
}

size_t c_tuntap_macosx_obj::read_from_tun(unsigned char * const data, size_t size) {
	_NOTREADY();
	_UNUSED(data);
	_UNUSED(size);
	return 0;
}

void c_tuntap_macosx_obj::async_receive_from_tun(unsigned char * const data,
                                                 size_t size,
                                                 const c_tuntap_base_obj::read_handler &handler) {
	_NOTREADY();
	_UNUSED(data);
	_UNUSED(size);
	_UNUSED(handler);
}

void c_tuntap_macosx_obj::set_tun_parameters(const std::array<unsigned char, 16> &binary_address,
                                             int prefix_len,
                                             uint32_t mtu) {
	_NOTREADY();
	_UNUSED(binary_address);
	_UNUSED(prefix_len);
	_UNUSED(mtu);
}

#endif // __MACH__

