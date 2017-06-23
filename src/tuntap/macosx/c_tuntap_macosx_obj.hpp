#ifndef C_MACOSX_TUNTAP_OBJ_HPP
#define C_MACOSX_TUNTAP_OBJ_HPP

#include <platform.hpp>
#ifdef ANTINET_macosx
#include "../base/tuntap_base.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>

class c_tuntap_macosx_obj final : public c_tuntap_base_obj {
	public:
		c_tuntap_macosx_obj(); ///< construct this object, throws if error

		size_t send_to_tun(const unsigned char *data, size_t size) override;

		size_t send_to_tun_separated_addresses(const unsigned char * const data, size_t size,
			const std::array<unsigned char, IPV6_LEN> &src_binary_address,
			const std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;

		size_t read_from_tun(unsigned char * const data, size_t size) override;
		size_t read_from_tun_separated_addresses(unsigned char * const data, size_t size,
			std::array<unsigned char, IPV6_LEN> &src_binary_address,
			std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;

		void async_receive_from_tun(unsigned char * const data,
		                            size_t size, const
		                            read_handler & handler) override;

		void set_tun_parameters (const std::array<unsigned char, IPV6_LEN> &binary_address,
		                         int prefix_len,
		                         uint32_t mtu) override;
	private:
		std::string m_ifr_name; ///< the name of interface. We want to set it in set_ipv6_address(), needed for set_mtu.
		const int m_tun_fd; ///< the unix file descriptor. -1 is closed (this should not happen in correct object)

		boost::asio::io_service m_io_service;
		boost::asio::posix::stream_descriptor m_tun_stream;

		/// create unix file descriptor
		int create_tun_fd();

		void set_ipv6_address(const std::array<uint8_t, IPV6_LEN> &binary_address, int prefixLen);
		void set_mtu(uint32_t mtu);

		static const int number_of_tested_cards = 100;
		static const int cards_testing_time = 5;
};

#endif // ANTINET_macosx

#endif // C_MACOSX_TUNTAP_OBJ_HPP
