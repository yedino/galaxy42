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

#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#include <boost/asio.hpp>
#include <ifdef.h>
#include <memory>
#include <windows.h>
#include <vector>
class c_tun_device_windows final : public c_tun_device {
	public:
		c_tun_device_windows();
		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		bool incomming_message_form_tun() override; ///< returns true if tun is readry for read
	private:
		boost::asio::io_service m_ioservice;
		std::unique_ptr<boost::asio::windows::stream_handle> m_stream_handle_ptr;
		std::array<uint8_t, 1500> m_buffer;
		size_t m_readed_bytes;
		HANDLE m_handle;

		std::vector<std::wstring> get_subkeys(HKEY hKey);
		std::wstring get_device_guid();
		std::wstring get_human_name(const std::wstring &guid);
		NET_LUID get_luid(const std::wstring &human_name);
		HANDLE get_device_handle();
};
#endif

#endif // C_TUN_DEVICE_HPP
