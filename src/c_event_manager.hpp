#ifndef C_EVENT_MANAGER_HPP
#define C_EVENT_MANAGER_HPP

//TODO (da2ce7) CIRCULAR HEADER INCLUDE
#include "c_tun_device.hpp"
#include "c_udp_wrapper.hpp"

class c_event_manager {
	public:
		virtual ~c_event_manager() = default;
		virtual void wait_for_event() = 0;
		virtual bool receive_udp_paket() = 0;
		virtual bool get_tun_packet() = 0;
		virtual void init()=0; ///< call this to finish init of the object

		virtual void init_without_tun(); ///< call this to finish if you want the FD to be disabled

	protected:
		bool m_tun_enabled=true; ///< should we use TUN (e.g. m_tun_fd in child class) or is TUN disabled for some reason
};



#ifdef __linux__
class c_tun_device_linux;
class c_udp_wrapper_linux;
#include <sys/select.h>
class c_event_manager_linux final : public c_event_manager {
	public:
		c_event_manager_linux(const c_tun_device_linux &tun_device, const c_udp_wrapper_linux &udp_wrapper);
		void wait_for_event() override;
		bool receive_udp_paket() override;
		virtual bool get_tun_packet() override;

		virtual void init() override; ///< call this to finish init of the object, call it:
		/// once the tun_device that we reference since constructor is now fully inited
		virtual void init_without_tun() override;

	private:
		std::reference_wrapper<const c_tun_device_linux> m_tun_device;

		int m_tun_fd;
		const int m_udp_socket;
		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input
};

// __linux__
#elif defined(_WIN32) || defined(__CYGWIN__) ||defined(__MACH__) || defined(__FreeBSD__)

#if defined(__CYGWIN__)
	#ifndef __USE_W32_SOCKETS
		#define __USE_W32_SOCKETS
	#endif
#endif

#include <functional>
#if defined(_WIN32) || defined(__CYGWIN__)
class c_tun_device_windows;
#elif defined(__MACH__)
class c_tun_device_apple;
#elif defined(__FreeBSD__)
class c_tun_device_freebsd;
#endif
class c_udp_wrapper_asio;

class c_event_manager_asio final : public c_event_manager {
public:
        #if defined(_WIN32) || defined(__CYGWIN__)
        c_event_manager_asio(c_tun_device_windows &tun_device, c_udp_wrapper_asio &udp_wrapper);
        #elif defined(__MACH__)
        c_event_manager_asio(c_tun_device_apple &tun_device, c_udp_wrapper_asio &udp_wrapper);
        #endif

	virtual void init() override; ///< call this to finish init of the object, call it:
	/// once the tun_device that we reference since constructor is now fully inited

	void wait_for_event() override;
	bool receive_udp_paket() override;
	virtual bool get_tun_packet() override;

private:
		int m_tun_fd;

        #if defined(_WIN32) || defined(__CYGWIN__)
	std::reference_wrapper<c_tun_device_windows> m_tun_device;
        #elif defined(__MACH__)
        std::reference_wrapper<c_tun_device_apple> m_tun_device;
        #endif
	std::reference_wrapper<c_udp_wrapper_asio> m_udp_device;
	bool m_tun_event;
	bool m_udp_event;
};

// _win32 || __cygwin__ || __MACH__
#else

#warning "using c_event_manager_empty = It can not work!"
class c_tun_device_empty;
class c_udp_wrapper_empty;
class c_event_manager_empty final : public c_event_manager {
	public:
		c_event_manager_empty() = default;
		c_event_manager_empty(const c_tun_device_empty &tun_device, const c_udp_wrapper_empty &udp_wrapper);
		void wait_for_event();
		bool receive_udp_paket();
		bool get_tun_packet();
};

// else
#endif

#endif // C_EVENT_MANAGER_HPP
