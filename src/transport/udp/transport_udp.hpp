#ifndef TRANSPORT_UDP_HPP
#define TRANSPORT_UDP_HPP

#include "../base/transp_base_obj.hpp"
#include <boost/asio.hpp>

class c_transport_udp final : public c_transport_base_obj {
	public:
		virtual void send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) override;

	private:
		boost::asio::io_service m_io_service;

};

#endif // TRANSPORT_UDP_HPP
