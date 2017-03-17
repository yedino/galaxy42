#ifndef C_MACOSX_TUNTAP_OBJ_HPP
#define C_MACOSX_TUNTAP_OBJ_HPP

#include <platform.hpp>
#ifdef ANTINET_macosx
#include "../base/tuntap_base.hpp"
#include <boost/asio.hpp>
#include <libs0.hpp>

// empty macosx class - NOT IMPLEMENTED YET
class c_tuntap_macosx_obj final : public c_tuntap_base_obj {
	public:
		c_tuntap_macosx_obj(); ///< construct this object, throws if error

		size_t send_to_tun(const unsigned char *data, size_t size) override;
		size_t read_from_tun(unsigned char * const data, size_t size) override;

		void async_receive_from_tun(unsigned char * const data,
		                            size_t size, const
		                            read_handler & handler) override;

		void set_tun_parameters (const std::array<unsigned char, 16> &binary_address,
		                         int prefix_len,
		                         uint32_t mtu) override;
};

#endif // ANTINET_macosx

#endif // C_MACOSX_TUNTAP_OBJ_HPP
