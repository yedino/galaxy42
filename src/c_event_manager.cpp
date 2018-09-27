#include "c_event_manager.hpp"
#include "tnetdbg.hpp"
#include <thread>
#include <chrono>




void c_event_manager::init_without_tun() {
	m_tun_enabled=false;
	pfp_throw_error( std::runtime_error("Option to run with missing TUN is not implemented for c_event_manager for current OS.") );
}

#ifdef __linux__
#include <limits>

c_event_manager_linux::c_event_manager_linux(const c_tun_device_linux &tun_device, const c_udp_wrapper_linux &udp_wrapper)
:
	m_tun_device(tun_device),
	m_tun_fd(-1),
	m_udp_socket(udp_wrapper.m_socket)
{
}

void c_event_manager_linux::init() {
	m_tun_fd = m_tun_device.get().get_tun_fd();
	if (m_tun_fd<0) pfp_throw_error(std::runtime_error("Trying to init event manager, but this tuntap device still doesn't have valid fd."));
	pfp_goal("Event manager will watch tuntap fd " << m_tun_fd);
}

void c_event_manager_linux::init_without_tun() {
	m_tun_fd=-1;
	m_tun_enabled=false;
}

void c_event_manager_linux::wait_for_event() {
	pfp_dbg3("Selecting. m_tun_fd="<<m_tun_fd);
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_udp_socket, &m_fd_set_data);

	if (m_tun_enabled) {
		if (m_tun_fd<0) pfp_throw_error(std::runtime_error("Trying to select, while tuntap fd is not ready in this class."));
		FD_SET(m_tun_fd, &m_fd_set_data);
	}
	auto fd_max = std::max(m_tun_fd, m_udp_socket);
	pfp_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	pfp_assert(fd_max >= 1);
	timeval timeout { 3 , 0 }; // http://pubs.opengroup.org/onlinepubs/007908775/xsh/systime.h.html
	pfp_dbg1("Selecting for fd_max="<<fd_max);
	auto select_result = select( fd_max+1, &m_fd_set_data, nullptr, nullptr, & timeout); // <--- blocks
	pfp_assert(select_result >= 0);

}

bool c_event_manager_linux::receive_udp_paket() {
	return FD_ISSET(m_udp_socket, &m_fd_set_data);
}

bool c_event_manager_linux::get_tun_packet() {
	return FD_ISSET(m_tun_fd, &m_fd_set_data);
}

// __linux__

#elif defined(_WIN32) || defined(__CYGWIN__) || defined (__MACH__)

#if defined(__MACH__)
c_event_manager_asio::c_event_manager_asio(c_tun_device_apple &tun_device, c_udp_wrapper_asio &udp_wrapper)
#else // _WIN32 || __CYGWIN__
c_event_manager_asio::c_event_manager_asio(c_tun_device_windows &tun_device, c_udp_wrapper_asio &udp_wrapper)
#endif
:
	m_tun_device(tun_device),
	m_udp_device(udp_wrapper),
	m_tun_event(false),
	m_udp_event(false)
{
}

void c_event_manager_asio::init() {
	#if defined(__MACH__)
		m_tun_fd = m_tun_device.get().get_tun_fd();
		pfp_goal("Event manager will watch tuntap fd " << m_tun_fd);
		if (m_tun_fd<0) pfp_throw_error(std::runtime_error("Trying to init event manager, but this tuntap device still doesn't have valid fd."));
	#else
		pfp_goal("Event manager will not watch tuntap using fd on this Operating System");
	#endif
}

void c_event_manager_asio::wait_for_event() {
	// TODO !!!
	// poll_one is not blocking function, possible 100% CPU usage
	// TODO use one io_service ojbect

	const auto time_start = std::chrono::steady_clock::now(); // time now
	const auto timeout = std::chrono::seconds( 3 ); // timeout of this select

	while(1) {

		if (m_tun_device.get().m_ioservice.poll_one() > 0) m_tun_event = true;
		else m_tun_event = false;

		if (m_udp_device.get().m_io_service.poll_one() > 0) m_udp_event = true;
		else m_udp_event = false;

		if (m_udp_event || m_tun_event) break; // got event

		const auto time_now = std::chrono::steady_clock::now(); // time now
		if (time_now > time_start + timeout) break; // no event - timeout

		// we wait - sleep a bit
                std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
	}
}

bool c_event_manager_asio::receive_udp_paket() {
	// if (m_udp_event) std::cout << "get udp packet" << std::endl;
	return m_udp_event;
}

bool c_event_manager_asio::get_tun_packet() {
	return m_tun_event;
}

// __win32 || __cygwin__
#elif defined(__MACH__)

// __mach__

#else

c_event_manager_empty::c_event_manager_empty(const c_tun_device_empty &tun_device, const c_udp_wrapper_empty &udp_wrapper) {
	pfp_UNUSED(tun_device);
	pfp_UNUSED(udp_wrapper);
}

void c_event_manager_empty::wait_for_event() { }
bool c_event_manager_empty::receive_udp_paket() { return false; }
bool c_event_manager_empty::get_tun_packet() { return false; }

// else
#endif
