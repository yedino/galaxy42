#ifndef TUNTAP_BASE_HPP
#define TUNTAP_BASE_HPP

#include <cstddef>
#include <functional>

class tuntap_base_obj {
	public:
		using read_handler = const std::function<void(const unsigned char *, std::size_t)> &;
		tuntap_base_obj();
		virtual ~tuntap_base_obj() = default;
		virtual size_t send_to_tun(const unsigned char *data, size_t size) = 0;
		virtual size_t read_from_tun(unsigned char * const data, size_t size) = 0;
		virtual size_t async_receive_from_tun(unsigned char * const data, size_t size, read_handler handler) = 0;
};

#endif // TUNTAP_BASE_HPP
