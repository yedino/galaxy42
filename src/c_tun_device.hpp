// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#ifndef C_TUN_DEVICE_HPP
#define C_TUN_DEVICE_HPP

#include <array>
#include <string>

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
		virtual bool incomming_message_form_tun() = 0;
};

#ifdef __linux__

class c_tun_device_linux final : public c_tun_device {
	public:
		c_tun_device_linux();
		void set_ipv6_address
			(const std::array<uint8_t, 16> &binary_address, int prefixLen) override;
		void set_mtu(uint32_t mtu);
		bool incomming_message_form_tun();

	private:
		int m_tun_fd;
};

#endif

#endif // C_TUN_DEVICE_HPP
