#pragma once
#ifndef cable_UDP_OBJ_HPP
#define cable_UDP_OBJ_HPP

#include "../base/cable_base_obj.hpp"
#include <boost/asio.hpp>
#include <list>
#include <mutex>
#include "libs0.hpp"

class c_cable_udp final : public c_cable_base_obj {
	public:
		c_cable_udp();
		void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) override;
		void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler) override;
		size_t receive_from(c_cable_base_addr & source, unsigned char * const data, size_t size) override;
		void async_receive_from(unsigned char * const data, size_t size, read_handler handler) override;
		void listen_on(c_cable_base_addr & local_address) override;

	private:
		boost::asio::ip::udp::socket m_read_socket;
		boost::asio::ip::udp::socket m_write_socket;
		std::mutex m_enpoint_list_mutex;
		std::list<boost::asio::ip::udp::endpoint> m_endpoint_list;

};

#endif // cable_UDP_HPP
