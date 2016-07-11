// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#include "c_tun_device.hpp"

#ifdef __linux__
#include <cassert>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "c_tnetdbg.hpp"
#include "../depends/cjdns-code/NetPlatform.h"
#include "cpputils.hpp"
c_tun_device_linux::c_tun_device_linux()
:
	m_tun_fd(open("/dev/net/tun", O_RDWR))
{
	assert(! (m_tun_fd<0) ); // TODO throw?
}

void c_tun_device_linux::set_ipv6_address
	(const std::array<uint8_t, 16> &binary_address, int prefixLen) {
	as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, static_cast<void *>(&ifr));
	if (errcode_ioctl < 0) throw std::runtime_error("ioctl error");
	assert(binary_address[0] == 0xFD);
	assert(binary_address[1] == 0x42);
	NetPlatform_addAddress(ifr.ifr_name, binary_address.data(), prefixLen, Sockaddr_AF_INET6);
}

void c_tun_device_linux::set_mtu(uint32_t mtu) {
	_UNUSED(mtu);
	_NOTREADY();
}

bool c_tun_device_linux::incomming_message_form_tun() {
	fd_set fd_set_data;
	FD_ZERO(&fd_set_data);
	FD_SET(m_tun_fd, &fd_set_data);
	timeval timeout { 0 , 500 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html
	auto select_result = select( m_tun_fd+1, &fd_set_data, nullptr, nullptr, & timeout); // <--- blocks
	_assert(select_result >= 0);
	if (FD_ISSET(m_tun_fd, &fd_set_data)) return true;
	else return false;
}

size_t c_tun_device_linux::read_from_tun(void *buf, size_t count) { // TODO throw if error
	ssize_t ret = read(m_tun_fd, buf, count); // <-- read data from TUN
	if (ret == -1) throw std::runtime_error("Read from tun error");
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

size_t c_tun_device_linux::write_to_tun(const void *buf, size_t count) { // TODO throw if error
	auto ret = write(m_tun_fd, buf, count);
	if (ret == -1) throw std::runtime_error("Write to tun error");
	assert (ret >= 0);
	return static_cast<size_t>(ret);
}

//#else

c_tun_device_empty::c_tun_device_empty() { }

void c_tun_device_empty::set_ipv6_address(const std::array<uint8_t, _Tp2> &binary_address, int prefixLen) {
	_UNUSED(binary_address);
	_UNUSED(prefixLen);
}

void c_tun_device_empty::set_mtu(uint32_t mtu) {
	_UNUSED(mtu);
}

bool c_tun_device_empty::incomming_message_form_tun() {
	return false;
}

size_t c_tun_device_empty::read_from_tun(void *buf, size_t count) {
	_UNUSED(buf);
	_UNUSED(count);
	return 0;
}

size_t c_tun_device_empty::write_to_tun(const void *buf, size_t count) {
	_UNUSED(buf);
	_UNUSED(count);
	return 0;
}

#endif // __linux__
