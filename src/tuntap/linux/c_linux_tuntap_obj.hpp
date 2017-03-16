#ifndef C_LINUX_TUNTAP_OBJ_HPP
#define C_LINUX_TUNTAP_OBJ_HPP

#ifdef __linux__

#include "../base/tuntap_base.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>

class c_linux_tuntap_obj final : public tuntap_base_obj {
	public:
		c_linux_tuntap_obj(); ///< construct this object, throws if error

		size_t send_to_tun(const unsigned char *data, size_t size) override;
		size_t read_from_tun(unsigned char * const data, size_t size) override;
		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;

		void set_tun_parameters
			(const std::array<unsigned char, 16> &binary_address, int prefix_len, uint32_t mtu) override;

	private:
		const int m_tun_fd; ///< the unix file descriptor. -1 is closed (this should not happen in correct object)
		boost::asio::io_service m_io_service;
		boost::asio::posix::stream_descriptor m_tun_stream;
};

#endif // __linux__

#endif // C_LINUX_TUNTAP_OBJ_HPP
