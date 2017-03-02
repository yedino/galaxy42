#pragma once
#ifndef TRANSPORT_UDP_OBJ_HPP
#define TRANSPORT_UDP_OBJ_HPP

#include "../base/transp_base_obj.hpp"
#include <boost/asio.hpp>

class c_transport_udp final : public c_transport_base_obj {
	public:
		void send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) override;
		size_t receive_from(c_transport_base_addr & source, unsigned char * const data, size_t size) override;
		void listen_on(c_transport_base_addr & local_address) override;

	private:
		boost::asio::io_service m_io_service;

};

#endif // TRANSPORT_UDP_HPP
