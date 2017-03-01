#pragma once
#ifndef TRANSPORT_UDP_ADDR_HPP
#define TRANSPORT_UDP_ADDR_HPP

#include "../base/transp_base_addr.hpp"
#include <boost/asio.hpp>

class c_transport_udp_addr final : public c_transport_base_addr {
	public:
		typedef boost::asio::ip::udp::endpoint t_addr; ///< actuall raw address. Instace of this will be in m_addrdata

		c_transport_udp_addr() = default;
		c_transport_udp_addr(const t_addr &endpoint);
		c_transport_udp_addr(const std::string &ip_string); ///< Converts string like "100.200.50.50:32000"
		void print(std::ostream & ostr) const override;
};

#endif // TRANSPORT_UDP_ADDR_HPP
