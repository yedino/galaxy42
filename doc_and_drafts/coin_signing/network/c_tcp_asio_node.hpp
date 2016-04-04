#ifndef C_TCP_ASIO_NODE_H
#define C_TCP_ASIO_NODE_H

#include "c_connection_base.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>

class c_tcp_asio_node final : public c_connection_base
{
	public:
		c_tcp_asio_node();
		virtual ~c_tcp_asio_node();
		void send(c_network_message && message) override;
		c_network_message receive() override;
	private:
		std::vector<std::unique_ptr<std::thread>> m_asio_threads;
		std::atomic<bool> m_stop_flag;
		boost::asio::io_service m_ioservice;
};

#endif // C_TCP_ASIO_NODE_H
