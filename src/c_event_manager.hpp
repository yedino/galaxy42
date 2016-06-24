#ifndef C_EVENT_MANAGER_HPP
#define C_EVENT_MANAGER_HPP


class c_event_manager {
	public:
		virtual ~c_event_manager() = default;
		virtual void wait_for_event() = 0;
		virtual bool receive_udp_paket() = 0;
		virtual bool get_tun_packet() = 0;
};

#ifdef __linux__
#include <sys/select.h>
class c_event_manager_linux final : public c_event_manager {
	private:
		int m_tun_fd; // TODO const?
		int m_udp_socket; // TODO const?
		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input
	public:
		c_event_manager_linux(int tun_fd, int udp_socket);
		void wait_for_event();
		bool receive_udp_paket();
		bool get_tun_packet();
};

#endif // __linux__

#endif // C_EVENT_MANAGER_HPP
