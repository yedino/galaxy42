#include "c_tcp_asio_node.hpp"

c_tcp_asio_node::c_tcp_asio_node()
:
	m_asio_threads(),
	m_stop_flag(false),
	m_ioservice(),
	m_recv_queue_ptr(std::make_shared<c_locked_queue<c_network_message>>())
{
	unsigned int number_of_threads = std::thread::hardware_concurrency();
	if (number_of_threads == 0) number_of_threads = 1;
	auto thread_lambda = [this]() {
		while(!m_stop_flag) {
			m_ioservice.run();
			m_ioservice.reset();
		}
	};
	for(unsigned int i = 0; i < number_of_threads; ++i) {
		m_asio_threads.emplace_back(new std::thread(thread_lambda)); // TODO make_unique
	}
}

c_tcp_asio_node::~c_tcp_asio_node() {
	m_stop_flag = true;
	m_ioservice.stop();
	for (auto &thread_ptr : m_asio_threads) {
		thread_ptr->join();
	}
}

void c_tcp_asio_node::send(c_network_message && message) {

}

c_network_message c_tcp_asio_node::receive() {

}


/************************************************************/

c_tcp_asio_node::connection::connection(boost::asio::io_service &io_service, const std::shared_ptr<c_locked_queue<c_network_message>> &msg_queue)
:
	m_socket(io_service)
{
}

void c_tcp_asio_node::connection::connect(const boost::asio::ip::tcp::endpoint &endpoint) {
	//m_socket.async_connect(endpoint, []()) // TODO
}

void c_tcp_asio_node::connection::send(std::string && message) {
}
