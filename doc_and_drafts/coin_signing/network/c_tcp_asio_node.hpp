#ifndef C_TCP_ASIO_NODE_H
#define C_TCP_ASIO_NODE_H

#include "c_connection_base.hpp"
#include "c_locked_queue.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <thread>
#include <vector>

class c_connection;

class c_tcp_asio_node final : public c_connection_base
{
	friend class c_connection;
	public:
		c_tcp_asio_node();
		~c_tcp_asio_node();
		void send(c_network_message && message) override;
		c_network_message receive() override; // TODO
	private:
		std::vector<std::unique_ptr<std::thread>> m_asio_threads;
		std::atomic<bool> m_stop_flag;
		boost::asio::io_service m_ioservice;
		c_locked_queue<c_network_message> m_recv_queue;

		std::mutex m_connection_map_mtx;
		std::map<boost::asio::ip::tcp::endpoint, std::shared_ptr<c_connection>> m_connection_map; ///< always use m_connection_map_mtx !!!

		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket_accept;

		void accept_handler(const boost::system::error_code& error); // TODO
};

class c_connection {
	public:
		c_connection(c_tcp_asio_node &node, const boost::asio::ip::tcp::endpoint &endpoint); ///< connect constructor
		c_connection(c_tcp_asio_node &node, boost::asio::ip::tcp::socket &&socket); ///< accept constructor // TODO
		void send(std::string && message);
		std::string receive(); // TODO

	private:
		c_tcp_asio_node &m_tcp_node;
		boost::asio::ip::tcp::socket m_socket;

		std::mutex m_streambuff_mtx;
		boost::asio::streambuf m_streambuff; ///< always lock m_streambuff_mtx before use
		std::ostream m_ostream; ///< always lock m_streambuff_mtx before use

		void write_handler(const boost::system::error_code &e, std::size_t length);

};

#endif // C_TCP_ASIO_NODE_H
