#include "c_event_manager.hpp"
#include "c_tnetdbg.hpp"

#ifdef __linux__
#include <limits>

c_event_manager_linux::c_event_manager_linux(const c_tun_device_linux &tun_device, const c_udp_wrapper_linux &udp_wrapper)
:
	m_tun_fd(tun_device.m_tun_fd),
	m_udp_socket(udp_wrapper.m_socket)
{
}

void c_event_manager_linux::wait_for_event() {
	_info("Selecting");
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_udp_socket, &m_fd_set_data);
	FD_SET(m_tun_fd, &m_fd_set_data);
	auto fd_max = std::max(m_tun_fd, m_udp_socket);
	_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	_assert(fd_max >= 1);
	timeval timeout { 3 , 0 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html
	auto select_result = select( fd_max+1, &m_fd_set_data, nullptr, nullptr, & timeout); // <--- blocks
	_assert(select_result >= 0);
}

bool c_event_manager_linux::receive_udp_paket() {
	return FD_ISSET(m_udp_socket, &m_fd_set_data);
}

bool c_event_manager_linux::get_tun_packet() {
	return FD_ISSET(m_tun_fd, &m_fd_set_data);
}

#else

c_event_manager_empty::c_event_manager_empty(const c_tun_device_empty &tun_device, const c_udp_wrapper_empty &udp_wrapper) {
	_UNUSED(tun_device);
	_UNUSED(udp_wrapper);
}

void c_event_manager_empty::wait_for_event() { }
bool c_event_manager_empty::receive_udp_paket() { return false; }
bool c_event_manager_empty::get_tun_packet() { return false; }

#endif // __linux__

#if defined(_WIN32) || defined(__CYGWIN__)
c_event_manager_windows::c_event_manager_windows(c_tun_device_windows &tun_device, c_udp_wrapper_windows &udp_wrapper)
:
	m_tun_device(tun_device),
	m_udp_device(udp_wrapper),
	m_tun_event(false),
	m_udp_event(false)
{
}

void c_event_manager_windows::wait_for_event() {
	// TODO !!!
	// poll_one is not blocking function, possible 100% CPU usage
	// TODO use one io_service ojbect
	if (m_tun_device.get().m_ioservice.poll_one() > 1) m_tun_event = true;
	else m_tun_event = false;
	if (m_udp_device.get().m_io_service.poll_one() > 1) m_udp_event = true;
	else m_tun_event = false;
}

bool c_event_manager_windows::receive_udp_paket() {
	return m_udp_event;
}

bool c_event_manager_windows::get_tun_packet() {
	return m_tun_event;
}

#endif
