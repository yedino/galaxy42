#include "c_tun_device.hpp"
#include <cassert>

#ifdef __linux__
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include "cpputils.hpp"
#include "cjdns-code/NetPlatform.h"
c_tun_device_linux::c_tun_device_linux()
:
	m_tun_fd(open("/dev/net/tun", O_RDWR))
{
	assert(! (m_tun_fd<0) ); // TODO throw?
}

void c_tun_device_linux::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen) {
	assert(binary_address[0] == 0xFD);
	assert(binary_address[1] == 0x42);
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	if (errcode_ioctl < 0) throw std::runtime_error("ioctl error");
	NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tun_device_linux::set_mtu(uint32_t mtu) {
	// TODO
	assert(false);
}

#endif
