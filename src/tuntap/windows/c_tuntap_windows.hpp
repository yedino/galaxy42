#ifndef C_TUNTAP_WINDOWS_HPP
#define C_TUNTAP_WINDOWS_HPP


#include "../base/tuntap_base.hpp"

#if defined(_WIN32) || defined(__CYGWIN__)

class c_tuntap_windows_obj final : c_tuntap_base_obj {
	public:
		c_tuntap_windows_obj();
		size_t send_to_tun(const unsigned char *data, size_t size) override;

		size_t read_from_tun(unsigned char * const data, size_t size) override;

		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;

		void set_tun_parameters
			(const std::array<unsigned char, 16> &binary_address, int prefix_len, uint32_t mtu) override;

	private:
		HANDLE m_handle;
		std::wstring m_guid;
		std::array<uint8_t, 6> m_mac_address;
		boost::asio::io_service m_ioservice;
		boost::asio::windows::stream_handle m_stream_handle; ///< boost handler to the TUN device

		std::vector<std::wstring> get_subkeys(HKEY hKey); ///< for windows registry
		std::wstring get_device_guid(); ///< technical name of the device
		std::wstring get_human_name(const std::wstring &guid);
		NET_LUID get_luid(const std::wstring &human_name);
		HANDLE get_device_handle();
		HANDLE open_tun_device(const std::wstring &guid); ///< returns opened handle for guid or INVALID_HANDLE_VALUE
		std::array<uint8_t, 6> get_mac(HANDLE handle); ///< get handle to opened device (returned by get_device_handle())

		class hkey_wrapper final {
			public:
				/**
				 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
				 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
				 */
				hkey_wrapper(HKEY hkey);
				~hkey_wrapper();
				HKEY &get();
				/**
				 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
				 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
				 */
				void set(HKEY new_hkey);
				void close();
			private:
				HKEY m_hkey;
				bool m_is_open;
  };
};

#endif // _WIN32 || __CYGWIN__

#endif // C_TUNTAP_WINDOWS_HPP
