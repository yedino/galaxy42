#ifndef C_TUN_DEVICE_WINDOWS_HPP
#define C_TUN_DEVICE_WINDOWS_HPP


#if defined(_WIN32) || defined(__CYGWIN__)
#include "c_tun_device.hpp"
#include <array>
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
		size_t read_from_tun(void *buf, size_t count) override;
		size_t write_to_tun(const void *buf, size_t count) override;
	private:
		boost::asio::io_service m_ioservice;
		std::array<uint8_t, 1500> m_buffer;
		size_t m_readed_bytes;
		HANDLE m_handle;
		std::unique_ptr<boost::asio::windows::stream_handle> m_stream_handle_ptr;

		std::vector<std::wstring> get_subkeys(HKEY hKey);
		std::wstring get_device_guid();
		std::wstring get_human_name(const std::wstring &guid);
		NET_LUID get_luid(const std::wstring &human_name);
		HANDLE get_device_handle();
		void handle_read(const boost::system::error_code& error, std::size_t length);
};
#endif

#endif // C_TUN_DEVICE_WINDOWS_HPP
