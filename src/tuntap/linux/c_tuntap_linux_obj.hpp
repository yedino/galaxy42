#ifndef C_LINUX_TUNTAP_OBJ_HPP
#define C_LINUX_TUNTAP_OBJ_HPP

#include <platform.hpp>
#ifdef ANTINET_linux

#include "../base/tuntap_base.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>
#include "../../test/mock_posix_stream_descriptor.hpp"

class c_tuntap_linux_obj final : public c_tuntap_base_obj {
	FRIEND_TEST(tuntap, send_to_tun);
	FRIEND_TEST(tuntap, send_to_tun_with_error);
	FRIEND_TEST(tuntap, send_to_tun_seperated);
	FRIEND_TEST(tuntap, read_from_tun);
	FRIEND_TEST(tuntap, read_from_tun_separated_addresses);
	public:
		c_tuntap_linux_obj(); ///< construct this object, throws if error

		size_t send_to_tun(const unsigned char *data, size_t size) override;
		size_t send_to_tun_separated_addresses(const unsigned char * const data, size_t size,
			const std::array<unsigned char, IPV6_LEN> &src_binary_address,
			const std::array<unsigned char, IPV6_LEN> &dst_binary_address);
		size_t read_from_tun(unsigned char * const data, size_t size) override;
		size_t read_from_tun_separated_addresses(unsigned char * const data, size_t size,
			std::array<unsigned char, IPV6_LEN> &src_binary_address,
			std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;
		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;
		void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) override;

	private:
		const int m_tun_fd; ///< the unix file descriptor. -1 is closed (this should not happen in correct object)
		boost::asio::io_service m_io_service;
#ifdef USE_MOCK
		using stream_type = mock::mock_posix_stream_descriptor;
#else
		using stream_type = boost::asio::posix::stream_descriptor;
#endif
		stream_type m_tun_stream;
};

#endif // ANTINET_linux

#endif // C_LINUX_TUNTAP_OBJ_HPP
