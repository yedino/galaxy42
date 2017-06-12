#ifndef TUNTAP_BASE_HPP
#define TUNTAP_BASE_HPP

#include <array>
#include <cstddef>
#include <functional>
#include <boost/system/system_error.hpp> // for the error_code
#include "someio.hpp"

#include "error_subtype.hpp"
#include "syserror_use.hpp"

#include "../depends/cjdns-code/syserr.h"
#include "../../../depends/cjdns-code/NetPlatform.h"

/// what is the size of headers that are removed when creating Merit
/// e.g. two times IPv6 address, since that is what we are removing from tuntap IPv6 packet
constexpr static size_t size_removed_from_merit = 2 * 16;

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
class c_tuntap_base_obj : protected c_someio {
	public:
		// the callback function that caller can provide
		using read_handler = std::function<void(const unsigned char *, std::size_t, const boost::system::error_code &)>;

		virtual ~c_tuntap_base_obj();
		/**
		 * @brief send_to_tun
		 * @param data pointer to block of data which ipv6 packet
		 * @param size size of block of data pointed by data pointer
		 * @return size of sended data, 0 if error
		 */
		virtual size_t send_to_tun(const unsigned char *data, size_t size) = 0; ///< blocking function
		// it seems that async send was slow.

		/**
		 * @brief send_to_tun_separated_addresses
		 * @param data pointer to block of data which ipv6 packet without src and dst address fields
		 * @param size size of block of data pointed by data pointer
		 * @param src_binary_address source address fiels
		 * @param dst_binary_address destination address field
		 * @return number of written bytes, if success returns size + src_binary_address.size() + dst_binary_address.size(),
		 * returns 0 if error
		 */
		virtual size_t send_to_tun_separated_addresses(const unsigned char * const data, size_t size,
			const std::array<unsigned char, IPV6_LEN> &src_binary_address,
			const std::array<unsigned char, IPV6_LEN> &dst_binary_address) = 0;

		/**
		 * @brief read_from_tun
		 * @param data pointer to block of data which will be filled by packet must be preallocated
		 * @param size size of block of data pointed by data pointer
		 * @return number of readed bytes, 0 if error
		 */
		virtual size_t read_from_tun(unsigned char * const data, size_t size) = 0; ///< blocking function

		/**
		 * @brief read_from_tun_separated_addresses
		 * @param data pointer to block of data which will be filled by ipv6 packet without src and dst address fields, must be preallocated
		 * @param size size of block of data pointed by data pointer
		 * @param src_binary_address source address filed
		 * @param dst_binary_address destination address field
		 * @return number of readed bytes without - 32 (dst size + src size == 32) or 0 if error
		 * if 0 is returned in data, src_binary_address or dst_binary_address may be trash data
		 */
		virtual size_t read_from_tun_separated_addresses(unsigned char * const data, size_t size,
			std::array<unsigned char, IPV6_LEN> &src_binary_address,
			std::array<unsigned char, IPV6_LEN> &dst_binary_address) = 0;

		// receive data to provided buffer, and then callback. Buffer must be valid up untill callback is called
		// then the callback is allowed to e.g. deallocate (or reuse) it.
		virtual void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) = 0;

		// sets the parameters of our tuntap
		virtual void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) = 0;
};

#endif // TUNTAP_BASE_HPP
