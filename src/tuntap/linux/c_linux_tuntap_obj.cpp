#ifdef __linux__

#include "c_linux_tuntap_obj.hpp"
#include "../../../depends/cjdns-code/NetPlatform.h"
#include "../../cpputils.hpp"
#include "../../utils/check.hpp"
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

c_linux_tuntap_obj::c_linux_tuntap_obj() :
	m_tun_fd(open("/dev/net/tun", O_RDWR)),
	m_io_service(),
	m_tun_stream(m_io_service, m_tun_fd)
{
	_try_sys(m_tun_fd != -1);
	_check_sys(m_tun_stream.is_open());
}

size_t c_linux_tuntap_obj::send_to_tun(const unsigned char *data, size_t size) {
	return m_tun_stream.write_some(boost::asio::buffer(data, size));
}

size_t c_linux_tuntap_obj::read_from_tun(unsigned char *const data, size_t size) {
	return m_tun_stream.read_some(boost::asio::buffer(data, size));
}

void c_linux_tuntap_obj::async_receive_from_tun(unsigned char *const data, size_t size,
	const tuntap_base_obj::read_handler & handler)
{
	auto asio_handler = [data, handler](const boost::system::error_code& error, std::size_t bytes_transferred) {
		handler(data, bytes_transferred, error);
	};
	return m_tun_stream.async_read_some(boost::asio::buffer(data, size), asio_handler);
}

void c_linux_tuntap_obj::set_tun_parameters(const std::array<uint8_t, 16> &binary_address, int prefix_len, uint32_t mtu) {
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	int errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	_check_sys(errcode_ioctl != -1);
	_check_extern(binary_address[0] == 0xFD);
	_check_extern(binary_address[1] == 0x42);
	NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefix_len, Sockaddr_AF_INET6);
	NetPlatform_setMTU(ifr.ifr_name, mtu);
	m_tun_stream.release();
	m_tun_stream.assign(m_tun_fd);
}

#endif // __linux__
