#ifndef TUNTAP_BASE_HPP
#define TUNTAP_BASE_HPP

#include <array>
#include <cstddef>
#include <functional>
#include <boost/system/system_error.hpp> // for the error_code

#include "error_subtype.hpp"
#include "syserror_use.hpp"

#include "../depends/cjdns-code/syserr.h"
#include "../../../depends/cjdns-code/NetPlatform.h"

std::string errno_to_string(int errno_copy); ///< Convert errno from C-lib into a string. Thread-safe function.

/// @return error-code-netplatform as a human (developer) string (not translated, it's tech detail)
/// this translates the field t_syserr.my_code only, see other function for more
std::string NetPlatform_error_code_to_string(int err);

/// @return error-code-netplatform as a human (developer) string (not translated, it's tech detail)
std::string NetPlatform_syserr_to_string(t_syserr syserr);

/**
 * Wrapper around the from-cjdns function;
 * Nicelly writes debug about this important function.
 * Errors are thrown as tuntap_error*
 */
void Wrap_NetPlatform_addAddress(const char* interfaceName,
                                 const uint8_t* address,
                                 int prefixLen,
                                 int addrFam);

/// Wrapper around the from-cjdns function:
void Wrap_NetPlatform_setMTU(const char* interfaceName,
                             uint32_t mtu);

constexpr size_t IPV6_LEN = 16;
/// Abstract class for API of tuntap driver, that allows to set own IP, and then send data to TUN/TAP, and read from it
/// Errors are either thrown, or reported to callere-provided handler (if the error happens async.)
class c_tuntap_base_obj {
	public:
		// the callback function that caller can provide
		using read_handler = std::function<void(const unsigned char *, std::size_t, const boost::system::error_code& error)>;

		virtual ~c_tuntap_base_obj();

		virtual size_t send_to_tun(const unsigned char *data, size_t size) = 0; ///< blocking function
		// it seems that async send was slow.

		virtual size_t read_from_tun(unsigned char * const data, size_t size) = 0; ///< blocking function

		/**
		 * @brief read_from_tun_without_header
		 * @param data pointer to block of data whitch will be filled by payload (i.e. udpv6 packet without ipv6 header), must be preallocated
		 * @param size size of block of data pointed by data pointer
		 * @param dst_binary_address will be filled by destination ipv6 from ipv6 packet header
		 * @param next_header next header filed from ipv6 header (i.e. 3a for icmpv6)
		 * @return number of readed bytes without ipv6 header size
		 */
		virtual size_t read_from_tun_without_header(unsigned char * const data,
			size_t size, std::array<unsigned char, IPV6_LEN> &dst_binary_address,
			uint8_t &next_header) = 0; ///< blocking function

		// receive data to provided buffer, and then callback. Buffer must be valid up untill callback is called
		// then the callback is allowed to e.g. deallocate (or reuse) it.
		virtual void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) = 0;

		// sets the parameters of our tuntap
		virtual void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) = 0;
};

#endif // TUNTAP_BASE_HPP
