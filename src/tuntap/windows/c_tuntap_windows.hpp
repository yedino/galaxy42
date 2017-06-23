#ifndef C_TUNTAP_WINDOWS_HPP
#define C_TUNTAP_WINDOWS_HPP


#include "../base/tuntap_base.hpp"
#include <platform.hpp>

#if defined(ANTINET_windows)
#include <ifdef.h>

class c_is_user_admin {
	public:
		c_is_user_admin();
		virtual ~c_is_user_admin() = default;
};

class c_tuntap_windows_obj final : public c_tuntap_base_obj, c_is_user_admin {
	public:
		c_tuntap_windows_obj();
		size_t send_to_tun(const unsigned char *data, size_t size) override;

		size_t send_to_tun_separated_addresses(const unsigned char * const data, size_t size,
			const std::array<unsigned char, IPV6_LEN> &src_binary_address,
			const std::array<unsigned char, IPV6_LEN> &dst_binary_address) override{_NOTREADY(); return 0;}

		size_t read_from_tun(unsigned char * const data, size_t size) override;

		size_t read_from_tun_separated_addresses(unsigned char * const data, size_t size,
			std::array<unsigned char, IPV6_LEN> &src_binary_address,
			std::array<unsigned char, IPV6_LEN> &dst_binary_address) override;

		void async_receive_from_tun(unsigned char * const data, size_t size, const read_handler & handler) override;

		void set_tun_parameters
			(const std::array<unsigned char, IPV6_LEN> &binary_address, int prefix_len, uint32_t mtu) override;

		HANDLE get_native_handle();
	private:
		std::wstring m_register_tun_path;
		std::wstring m_guid; // https://msdn.microsoft.com/en-us/library/windows/desktop/aa368767(v=vs.85).aspx
		HANDLE m_handle;
		static constexpr size_t mac_address_size = 6;
		std::array<uint8_t, mac_address_size> m_mac_address;
		boost::asio::io_service m_ioservice;
		boost::asio::windows::stream_handle m_stream_handle; ///< boost handler to the TUN device

		std::vector<std::wstring> get_subkeys(HKEY hKey); ///< for windows registry
		std::wstring get_device_guid(); ///< technical name of the device
		std::wstring get_human_name(const std::wstring &guid);
		NET_LUID get_luid(const std::wstring &human_name);
		HANDLE get_device_handle();
		HANDLE open_tun_device(const std::wstring &guid) noexcept; ///< returns opened handle for guid or INVALID_HANDLE_VALUE
		std::array<uint8_t, mac_address_size> get_mac(HANDLE handle); ///< get handle to opened device (returned by get_device_handle())
		void set_mtu(uint32_t mtu);

		class hkey_wrapper final {
			public:
				/**
				 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
				 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
				 */
				hkey_wrapper(HKEY hkey);
				~hkey_wrapper(); ///< call close() method
				HKEY &get(); ///< throws std::runtime_error if HKEY object is not open
				/**
				 * hkey must have been opened by the RegCreateKeyEx, RegCreateKeyTransacted,
				 * RegOpenKeyEx, RegOpenKeyTransacted, or RegConnectRegistry function.
				 * multiple calling this function with opened HKEY objects is safe (old hkeys will be closed via close() method)
				 */
				void set(HKEY new_hkey);
				void close(); ///< close HKEY object via RegClose_key() function, multiple calling this function is safe
			private:
				HKEY m_hkey;
				bool m_is_open;
  };
};

#endif //ANTINET_windows

#endif // C_TUNTAP_WINDOWS_HPP
