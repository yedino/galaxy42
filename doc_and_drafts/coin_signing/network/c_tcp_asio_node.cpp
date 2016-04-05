#include "c_tcp_asio_node.hpp"

using namespace boost::asio;

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
	c_network_message msg(std::move(message));
	ip::address_v4 ip_addr = ip::address_v4::from_string(msg.address_ip); // TODO throw if bad address
	ip::tcp::endpoint endpoint(ip_addr, msg.port);

	std::lock_guard<std::mutex> lg(m_connection_map_mtx);
	auto it = m_connection_map.find(endpoint); // find destination connection
	if (it == m_connection_map.end()) { // not found connection, create new
		m_connection_map.emplace(endpoint, std::make_shared<c_connection>(*this));
	}
	assert(!m_connection_map.empty());
	m_connection_map.at(endpoint)->send(std::move(msg.data));
}

c_network_message c_tcp_asio_node::receive() {

}


/************************************************************/

c_connection::c_connection(c_tcp_asio_node &node)
:
	m_tcp_node(node),
	m_socket(node.m_ioservice)
{
	// TODO connect()
}

void c_connection::connect(const boost::asio::ip::tcp::endpoint &endpoint) {
	//m_socket.async_connect(endpoint, []()) // TODO
}

void c_connection::send(std::string && message) {
}
