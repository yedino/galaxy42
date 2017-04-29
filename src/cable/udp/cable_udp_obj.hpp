#pragma once
#ifndef cable_UDP_OBJ_HPP
#define cable_UDP_OBJ_HPP

#include "../base/cable_base_obj.hpp"
#include <boost/asio.hpp>
#include <list>
#include <mutex>
#include "libs0.hpp"
#include "../asiocable.hpp"
#include "../asio_ioservice_manager.hpp"

#include <cable/selector.hpp>
#include <cable/udp/cable_udp_addr.hpp>

class c_cable_udp final : public c_asiocable {
	public:
		/**
		 * create UDP cable to recv/send, using one source address (can be just port)
		 * @param iomanager - use this manager
		 * @param source_addr - we will send from this source address. Can be general ANY address e.g. 0.0.0.0:9042(?)
		 * to enforce port, or a specific one to enforce entire source address.
		*/
		c_cable_udp(shared_ptr<c_asioservice_manager> iomanager, const c_card_selector &source_addr);

		void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) override;
		void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler) override;

		size_t receive_from(c_card_selector & source, unsigned char * const data, size_t size) override;
		size_t receive_from(c_cable_udp_addr & source, unsigned char * const data, size_t size) ;
		void async_receive_from(unsigned char * const data, size_t size, read_handler handler) override;

		void listen_on(const c_card_selector & local_address) override;

		virtual void stop_threadsafe() override;

	private:
		boost::asio::ip::udp::socket m_read_socket;
		boost::asio::ip::udp::socket m_write_socket;
		Mutex m_enpoint_list_mutex;
		std::list<boost::asio::ip::udp::endpoint> m_endpoint_list;

};

#endif // cable_UDP_HPP
