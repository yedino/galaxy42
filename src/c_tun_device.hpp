// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#ifndef C_TUN_DEVICE_HPP
#define C_TUN_DEVICE_HPP

#include <array>
#include <string>
#include "c_event_manager.hpp"

/**
 * @brief The c_tun_device class
 * Abstract class to represent tun device
 */
class c_tun_device {
	public:
		virtual ~c_tun_device() = default;
		virtual void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) = 0;
		virtual void set_mtu(uint32_t mtu) = 0;
		virtual bool incomming_message_form_tun() = 0; ///< returns true if tun is readry for read
		virtual size_t read_from_tun(void *buf, size_t count) = 0;
		virtual size_t write_to_tun(const void *buf, size_t count) = 0;
};

#ifdef __linux__

class c_tun_device_linux final : public c_tun_device {
	friend class c_event_manager_linux;
	public:
		c_tun_device_linux();
		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		void set_mtu(uint32_t mtu) override;
		bool incomming_message_form_tun() override;
		size_t read_from_tun(void *buf, size_t count) override;
		size_t write_to_tun(const void *buf, size_t count) override;

	private:
		const int m_tun_fd;
};

// __linux__
#elif defined(_WIN32) || defined(__CYGWIN__)

#if defined(__CYGWIN__)
	#ifndef __USE_W32_SOCKETS
		#define __USE_W32_SOCKETS
	#endif
#endif

#include "c_tun_device.hpp"
#include "c_ndp.hpp"
#include <array>
#include <boost/asio.hpp>
#include <ifdef.h>
#include <memory>
#include <windows.h>
#include <vector>

class c_tun_device_windows final : public c_tun_device {
	friend class c_event_manager_windows;
	//friend class c_ndp;
public:
	c_tun_device_windows();
	void set_ipv6_address
		(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
	void set_mtu(uint32_t mtu) {}; // TODO
	bool incomming_message_form_tun() override; ///< returns true if tun is ready for read
	size_t read_from_tun(void *buf, size_t count) override; ///< count must be <= size of buffer in buf! otherwise UB
	size_t write_to_tun(const void *buf, size_t count) override; ///< count must be <= the size of buffer in buf! otherwise UB

																 //constexpr int m_buffer_size = 9000; ///< the buffer size. This can affect size of MTU that this TUN can offer maybe

private:
	std::wstring m_guid;
	boost::asio::io_service m_ioservice;
	std::array<uint8_t, 9000> m_buffer;

	size_t m_readed_bytes; ///< currently read bytes that await in m_buffer

	HANDLE m_handle; ///< windows handle to the TUN device
	std::unique_ptr<boost::asio::windows::stream_handle> m_stream_handle_ptr; ///< boost handler to the TUN device
	std::array<uint8_t, 6> m_mac_address;

	std::vector<std::wstring> get_subkeys(HKEY hKey); ///< for windows registry
	std::wstring get_device_guid(); ///< technical name of the device
	std::wstring get_human_name(const std::wstring &guid);
	NET_LUID get_luid(const std::wstring &human_name);
	HANDLE get_device_handle();
	HANDLE open_tun_device(const std::wstring &guid); // returns opened handle for guid or INVALID_HANDLE_VALUE
	std::array<uint8_t, 6> get_mac(HANDLE handle); // get handle to opened device (returned by get_device_handle())

	void handle_read(const boost::system::error_code& error, std::size_t length); ///< ASIO handler
};

// _win32 || __cygwin__
#else

class c_tun_device_empty final : public c_tun_device {
	public:
		c_tun_device_empty();
		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		void set_mtu(uint32_t mtu) override;
		bool incomming_message_form_tun() override;
		size_t read_from_tun(void *buf, size_t count) override;
		size_t write_to_tun(const void *buf, size_t count) override;
};

// else
#endif

#endif // C_TUN_DEVICE_HPP
