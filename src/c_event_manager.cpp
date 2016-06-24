#include "c_event_manager.hpp"
#include "c_tnetdbg.hpp"

#ifdef __linux__
#include <limits>
c_event_manager_linux::c_event_manager_linux(int tun_fd, int udp_socket)
:
	m_tun_fd(tun_fd),
	m_udp_socket(udp_socket)
{
	FD_ZERO(& m_fd_set_data);
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

#endif // __linux__
