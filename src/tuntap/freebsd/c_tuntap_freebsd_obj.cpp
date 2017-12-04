#include "c_tuntap_freebsd_obj.hpp"
#ifdef __FreeBSD__


/////////////////////////////////////////////////////////////////////

c_tuntap_freebsd_obj::c_tuntap_freebsd_obj() :
{
}

size_t c_tuntap_freebsd_obj::send_to_tun(const unsigned char *data, size_t size) {
	return 0;
}

size_t c_tuntap_freebsd_obj::send_to_tun_separated_addresses(const unsigned char *const data, size_t size,
	return 0;
}

size_t c_tuntap_freebsd_obj::read_from_tun(unsigned char *const data, size_t size) {
	return 0;
}

size_t c_tuntap_freebsd_obj::read_from_tun_separated_addresses(unsigned char *const data, size_t size,
	std::array<unsigned char, IPV6_LEN> &src_binary_address,
	std::array<unsigned char, IPV6_LEN> &dst_binary_address) {
	    return 0;
	
}

void c_tuntap_freebsd_obj::async_receive_from_tun(unsigned char *const data,
                                                size_t size,
                                                const c_tuntap_base_obj::read_handler & handler) {

	
	return 0; 
}

void c_tuntap_freebsd_obj::set_tun_parameters(const std::array<unsigned char, IPV6_LEN> &binary_address,
                                            int prefix_len,
                                            uint32_t mtu) {

}

#endif // ANTINET_linux
