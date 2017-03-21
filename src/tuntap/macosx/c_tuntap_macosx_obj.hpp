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
		size_t read_from_tun(unsigned char * const data, size_t size) override;

		void async_receive_from_tun(unsigned char * const data,
		                            size_t size, const
		                            read_handler & handler) override;

		void set_tun_parameters (const std::array<unsigned char, 16> &binary_address,
		                         int prefix_len,
		                         uint32_t mtu) override;
	private:
		const int m_tun_fd; ///< the unix file descriptor. -1 is closed (this should not happen in correct object)
		std::string m_ifr_name; ///< the name of interface. We want to set it in set_ipv6_address(), needed for set_mtu.

		boost::asio::io_service m_io_service;
		boost::asio::posix::stream_descriptor m_tun_stream;

		/// create unix file descriptor
		int create_tun_fd();

		void set_ipv6_address(const std::array<uint8_t, 16> &binary_address, int prefixLen);
		void set_mtu(uint32_t mtu);

		static const int number_of_tested_cards = 100;
		static const int cards_testing_time = 5;
};

#endif // ANTINET_macosx

#endif // C_MACOSX_TUNTAP_OBJ_HPP
