#include "c_tcp_asio_node.hpp"

c_tcp_asio_node::c_tcp_asio_node()
:
	m_asio_threads(),
	m_stop_flag(false),
	m_ioservice()
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
		m_asio_threads.emplace_back(new std::thread(thread_lambda));
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
