#ifndef C_TCP_ASIO_NODE_H
#define C_TCP_ASIO_NODE_H

#include "c_connection_base.hpp"
#include "c_locked_queue.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <condition_variable>
#include <map>
#include <memory>
#include <thread>
#include <vector>

class c_tcp_asio_node final : public c_connection_base
{
	public:
		c_tcp_asio_node();
		~c_tcp_asio_node();
		void send(c_network_message && message) override; // TODO
		c_network_message receive() override; // TODO
	private:
		std::vector<std::unique_ptr<std::thread>> m_asio_threads;
		std::atomic<bool> m_stop_flag;
		boost::asio::io_service m_ioservice;
		std::shared_ptr<c_locked_queue<c_network_message>> m_recv_queue_ptr;

		class connection {
			public:
				connection(boost::asio::io_service &io_service, const std::shared_ptr<c_locked_queue<c_network_message>> &msg_queue);
				void send(std::string && message); // TODO
				std::string receive(); // TODO

			private:
				boost::asio::ip::tcp::socket m_socket;
				std::mutex m_streambuff_mtx;
				boost::asio::streambuf m_streambuff;

				void connect(const boost::asio::ip::tcp::endpoint &endpoint); // TODO
		};

		std::map<boost::asio::ip::tcp::endpoint, connection> m_connection_map;
};

#endif // C_TCP_ASIO_NODE_H
