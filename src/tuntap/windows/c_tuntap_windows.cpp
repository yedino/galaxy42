#include "c_tuntap_windows.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)


c_tuntap_windows::c_tuntap_windows() {
}

size_t c_tuntap_windows::send_to_tun(const unsigned char *data, size_t size) {
	return 0;
}

size_t c_tuntap_windows::read_from_tun(unsigned char *const data, size_t size) {
	return 0;
}

void c_tuntap_windows::async_receive_from_tun(unsigned char *const data, size_t size, const c_tuntap_base_obj::read_handler &handler) {
}

void c_tuntap_windows::set_tun_parameters(const std::array<unsigned char, _Tp2> &binary_address, int prefix_len, uint32_t mtu) {
}


#endif // _WIN32 || __CYGWIN__
