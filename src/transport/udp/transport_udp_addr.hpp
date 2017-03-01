#ifndef TRANSPORT_UDP_ADDR_HPP
#define TRANSPORT_UDP_ADDR_HPP

#include "../base/transp_base_addr.hpp"
#include <boost/asio.hpp>

class c_transport_udp_addr final : public c_transport_base_addr {
	public:
		c_transport_udp_addr() = default;
		c_transport_udp_addr(const boost::asio::ip::udp::endpoint &endpoint);
		c_transport_udp_addr(const std::string &ip_string); ///< Converts string like "100.200.50.50:32000"
		void print(std::ostream & ostr) const override;
		const boost::asio::ip::udp::endpoint &get_native();
	private:
		boost::asio::ip::udp::endpoint m_endpoint;
};

#endif // TRANSPORT_UDP_ADDR_HPP
