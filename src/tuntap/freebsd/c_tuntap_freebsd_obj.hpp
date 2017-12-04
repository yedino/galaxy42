#ifndef C_FREEBSD_TUNTAP_OBJ_HPP
#define C_FREEBSD_TUNTAP_OBJ_HPP

#include <platform.hpp>
#ifdef __FreeBSD__

#include "../base/tuntap_base.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>
#include "../../test/mock_posix_stream_descriptor.hpp"
#include "../../test/mock_tuntap_system_functions.hpp"
#include "../../i_tuntap_system_functions.hpp"


class c_tuntap_freebsd_obj final : public c_tuntap_base_obj {
		public:
		c_tuntap_freebsd_obj(); ///< construct this object, throws if error

		size_t send_to_tun(const unsigned char *data, size_t size) override;
		size_t send_to_tun_separated_addresses(const unsigned char * const data, size_t size,
			const std::array<unsigned char, IPV6_LEN> &src_binary_address,
			const std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;
		size_t read_from_tun(unsigned char * const data, size_t size) override;
		size_t read_from_tun_separated_addresses(unsigned char * const data, size_t size,
			std::array<unsigned char, IPV6_LEN> &src_binary_address,
			std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;
		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;
		void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) override;

	private:
	
};

#endif // ANTINET_linux

#endif // C_LINUX_TUNTAP_OBJ_HPP
