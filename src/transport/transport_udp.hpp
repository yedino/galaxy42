#ifndef TRANSPORT_UDP_HPP
#define TRANSPORT_UDP_HPP

#include "base/transp_base_obj.hpp"
#include <boost/asio.hpp>

class c_transport_udp final : public c_transport_base_obj {
	public:
		void send_data(boost::any dest, const char *data, size_t size_of_data) override;
	private:
		boost::asio::io_service m_io_service;

};

#endif // TRANSPORT_UDP_HPP
