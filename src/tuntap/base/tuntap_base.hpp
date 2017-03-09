#ifndef TUNTAP_BASE_HPP
#define TUNTAP_BASE_HPP

#include <array>
#include <cstddef>
#include <functional>

class tuntap_base_obj {
	public:
		using read_handler = const std::function<void(const unsigned char *, std::size_t)> &;
		virtual ~tuntap_base_obj() = default;
		virtual size_t send_to_tun(const unsigned char *data, size_t size) = 0;
		virtual size_t read_from_tun(unsigned char * const data, size_t size) = 0;
		virtual size_t async_receive_from_tun(unsigned char * const data, size_t size, read_handler handler) = 0;
		virtual void set_tun_parameters
			(const std::array<uint8_t, 16> &binary_address, int prefixLen, uint32_t mtu) = 0;
};

#endif // TUNTAP_BASE_HPP
