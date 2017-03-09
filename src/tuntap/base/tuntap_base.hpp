#ifndef TUNTAP_BASE_HPP
#define TUNTAP_BASE_HPP

#include <array>
#include <cstddef>
#include <functional>
#include <boost/system/system_error.hpp> // for the error_code

/// Abstract class for API of tuntap driver, that allows to set own IP, and then send data to TUN/TAP, and read from it
/// Errors are either thrown, or reported to callere-provided handler (if the error happens async.)
class tuntap_base_obj {
	public:
		// the callback function that caller can provide
		using read_handler = std::function<void(const unsigned char *, std::size_t, const boost::system::error_code& error)>;

		virtual ~tuntap_base_obj() = default;

		virtual size_t send_to_tun(const unsigned char *data, size_t size) = 0; ///< blocking function
		// it seems that async send was slow.

		virtual size_t read_from_tun(unsigned char * const data, size_t size) = 0; ///< blocking function

		// receive data to provided buffer, and then callback. Buffer must be valid up untill callback is called
		// then the callback is allowed to e.g. deallocate (or reuse) it.
		virtual void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) = 0;

		// sets the parameters of our tuntap
		virtual void set_tun_parameters
			(const std::array<uint8_t, 16> &binary_address, int prefix_len, uint32_t mtu) = 0;
};

#endif // TUNTAP_BASE_HPP
