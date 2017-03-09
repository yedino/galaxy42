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
	_check_sys(m_tun_fd != -1);
	_check_sys(m_tun_stream.is_open());
}

void c_linux_tuntap_obj::set_tun_parameters(const std::array<uint8_t, 16> &binary_address, int prefix_len, uint32_t mtu) {
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN;
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	std::cout << "iface name " << ifr.ifr_name << '\n';
	std::cout << "IFNAMSIZ " << IFNAMSIZ << '\n';
	int errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	_check_sys(errcode_ioctl != -1);
	_check_extern(binary_address[0] == 0xFD);
	_check_extern(binary_address[1] == 0x42);
	NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefix_len, Sockaddr_AF_INET6);
	NetPlatform_setMTU(ifr.ifr_name, mtu);
	m_tun_stream.release();
	m_tun_stream.assign(m_tun_fd);
}
