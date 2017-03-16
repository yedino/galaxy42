#ifndef C_TUNTAP_WINDOWS_HPP
#define C_TUNTAP_WINDOWS_HPP


#include "../base/tuntap_base.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)

class c_tuntap_windows final : c_tuntap_base_obj {
	public:
		c_tuntap_windows();
		size_t send_to_tun(const unsigned char *data, size_t size) override;

		size_t read_from_tun(unsigned char * const data, size_t size) override;

		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;

		void set_tun_parameters
			(const std::array<unsigned char, 16> &binary_address, int prefix_len, uint32_t mtu) override;
};

#endif // _WIN32 || __CYGWIN__

#endif // C_TUNTAP_WINDOWS_HPP
