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

#endif // C_TUN_DEVICE_HPP
