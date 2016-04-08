#include "c_tcp_asio_node.hpp"

#include <functional>

using namespace boost::asio;
using namespace asio_node;

c_tcp_asio_node::c_tcp_asio_node(unsigned int port)
:
	m_asio_threads(),
	m_stop_flag(false),
	m_ioservice(),
	m_recv_queue(),
	m_acceptor(m_ioservice, ip::tcp::endpoint(ip::tcp::v4(), port)),
	m_socket_accept(m_ioservice)
{
	_dbg_mtx("c_tcp_asio_node constructor");
	unsigned int number_of_threads = std::thread::hardware_concurrency();
	if (number_of_threads == 0) number_of_threads = 1;
	_dbg_mtx("create " << number_of_threads << " threads for asio");
	auto thread_lambda = [this]() {
		while(!m_stop_flag) {
			_dbg_mtx(this << " io_service loop");
			m_ioservice.run();
			m_ioservice.reset();
		}
		_dbg_mtx(this << " m_ioservice stopped " << m_ioservice.stopped());
		_dbg_mtx(this << " end of thread lambda");
	};
	for(unsigned int i = 0; i < number_of_threads; ++i) {
		m_asio_threads.emplace_back(new std::thread(thread_lambda)); // TODO make_unique
	}

	m_acceptor.async_accept(m_socket_accept, std::bind(&c_tcp_asio_node::accept_handler, this, std::placeholders::_1));
}

c_tcp_asio_node::~c_tcp_asio_node() {
	_dbg_mtx(this << " c_tcp_asio_node destructor");
	m_stop_flag = true;
	while (!m_ioservice.stopped()) {
		m_ioservice.stop();
		_dbg_mtx(this << " io_service stop loop");
	}
	_dbg_mtx(this << " io_service stopped " << m_ioservice.stopped());
	m_acceptor.close();
	m_socket_accept.close();
	for (auto &thread_ptr : m_asio_threads) {
		thread_ptr->join();
	}
}

void c_tcp_asio_node::send(c_network_message && message) {
	c_network_message msg(std::move(message));
	ip::address_v4 ip_addr = ip::address_v4::from_string(msg.address_ip); // TODO throw if bad address
	ip::tcp::endpoint endpoint(ip_addr, msg.port); // generate endpoint from message

	std::lock_guard<std::mutex> lg(m_connection_map_mtx);
	auto it = m_connection_map.find(endpoint); // find destination connection
	if (it == m_connection_map.end()) { // not found connection, create new
		m_connection_map.emplace(endpoint, c_connection::s_create_connection(*this, endpoint));
	}
	assert(!m_connection_map.empty());
	m_connection_map.at(endpoint)->send(std::move(msg.data)); // send raw data
}

c_network_message c_tcp_asio_node::receive() {
	c_network_message message;
	std::lock_guard<std::recursive_mutex> lg(m_recv_queue.get_mutex());
	if (m_recv_queue.empty()) {
		return message;
	}
	message = m_recv_queue.pop();
	return message;
}


void c_tcp_asio_node::accept_handler(const boost::system::error_code &error) {
	_dbg_mtx("accept handler");
	if (error) {
		_dbg_mtx("error: " << error.message());
		return;
	}
	auto endpoint = m_socket_accept.remote_endpoint();
	std::unique_lock<std::mutex> lg(m_connection_map_mtx);
	//m_connection_map.emplace(endpoint, std::make_shared<c_connection>(*this, std::move(m_socket_accept)));
	m_connection_map.emplace(endpoint, c_connection::s_create_connection(*this, std::move(m_socket_accept)));
	lg.unlock();
	m_acceptor.async_accept(m_socket_accept, std::bind(&c_tcp_asio_node::accept_handler, this, std::placeholders::_1));
	_dbg_mtx("accept handler end");
}


/************************************************************/

std::shared_ptr<c_connection> c_connection::s_create_connection(c_tcp_asio_node &node, const ip::tcp::endpoint &endpoint) {
	std::shared_ptr<c_connection> connection = std::make_shared<c_connection>(node, endpoint);
	connection->m_myself = connection->shared_from_this();
	return connection;
}

std::shared_ptr< c_connection > c_connection::s_create_connection(c_tcp_asio_node &node, ip::tcp::socket && socket) {
	std::shared_ptr<c_connection> connection = std::make_shared<c_connection>(node, std::move(socket));
	connection->m_myself = connection->shared_from_this();
	return connection;
}


c_connection::c_connection(c_tcp_asio_node &node, const boost::asio::ip::tcp::endpoint &endpoint)
:
	m_myself(nullptr),
	m_tcp_node(node),
	m_socket(node.m_ioservice),
	m_streambuff(),
	m_ostream(&m_streambuff),
	m_read_size(),
	m_streambuff_in()
{
	_dbg_mtx("wait for connect");
	m_socket.connect(endpoint); // TODO throw if error
	_dbg_mtx("connected");
	boost::asio::ip::tcp::no_delay option;
	m_socket.set_option(option);
	m_socket.async_read_some(buffer(&m_read_size, sizeof(m_read_size)),
							std::bind(&c_connection::read_size_handler, this, std::placeholders::_1, std::placeholders::_2));
}

c_connection::c_connection(c_tcp_asio_node &node, ip::tcp::socket && socket)
:
	m_myself(nullptr),
	m_tcp_node(node),
	m_socket(std::move(socket)),
	m_streambuff(),
	m_ostream(&m_streambuff),
	m_read_size(),
	m_streambuff_in()
{
	boost::asio::ip::tcp::no_delay option;
	m_socket.set_option(option);
	// start read size
	m_socket.async_read_some(buffer(&m_read_size, sizeof(m_read_size)),
							std::bind(&c_connection::read_size_handler, this, std::placeholders::_1, std::placeholders::_2));
}

c_connection::~c_connection() {
	_dbg_mtx("");
	if (m_socket.is_open()) {
		_dbg_mtx("close connection socket");
		m_socket.shutdown(ip::tcp::socket::socket_base::shutdown_both);
		m_socket.close();
	}
}

void c_connection::send(std::string && message) {
	const uint32_t size_of_message = message.size();
	std::string msg(std::move(message));
	_dbg_mtx("msg.size() = " << msg.size());
	_dbg_mtx("size_of_message = " << size_of_message);
	assert(msg.size() == size_of_message);
	std::unique_lock<std::mutex> lg(m_streambuff_mtx);
	m_ostream.write(reinterpret_cast<const char *>(&size_of_message), sizeof(size_of_message));
	m_ostream.write(msg.data(), msg.size());
	lg.unlock();
	m_socket.async_write_some(buffer(m_streambuff.data(), m_streambuff.size()),
							std::bind(&c_connection::write_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void c_connection::write_handler(const boost::system::error_code &error, std::size_t length) {
	_dbg_mtx("write " << length << " bytes");
	if (error) { // error
		_dbg_mtx("error: " << error.message());
		delete_me();
		return;
	}
	std::lock_guard<std::mutex> lg(m_streambuff_mtx);
	m_streambuff.consume(length); // remove sended data from stream
	if (m_streambuff.size() > 0) {
		m_socket.async_write_some(buffer(m_streambuff.data(), m_streambuff.size()),
							std::bind(&c_connection::write_handler, this, std::placeholders::_1, std::placeholders::_2));
	}
	_dbg_mtx("end");
}

void c_connection::read_size_handler(const boost::system::error_code &error, size_t length) {
	_dbg_mtx("length " << length);
	if (error) {
		_dbg_mtx("error: " << error.message());
		delete_me();
		return; // TODO close connection
	}
	assert(m_read_size > 0); // TODO throw?

	_dbg_mtx("wait for " << m_read_size << " bytes");
	async_read(m_socket, m_streambuff_in,
							transfer_exactly(m_read_size),
							std::bind(&c_connection::read_data_handler, this, std::placeholders::_1, std::placeholders::_2));
}

void c_connection::read_data_handler(const boost::system::error_code &error, size_t length) {
	_dbg_mtx("***************************chunk");
	_dbg_mtx("chunk size " << length);
	if (error || length == 0) {
		_dbg_mtx("error: " << error.message());
		delete_me();
		return; // TODO close connection
	}
	_dbg_mtx("m_read_size " << m_read_size);
	_dbg_mtx("length " << length);
	// generate c_network_message
	c_network_message network_message;
	network_message.data.reserve(length);
	_dbg_mtx("streambuff size " << m_streambuff_in.size());
	streambuf::const_buffers_type buf = m_streambuff_in.data();
	std::copy(buffers_begin(buf), buffers_begin(buf) + length, std::back_inserter(network_message.data));
	m_streambuff_in.consume(length);
	_dbg_mtx("network_message.data.size() " << network_message.data.size());
	//assert(network_message.data.size() == m_read_size);
	auto endpoint = m_socket.remote_endpoint();
	network_message.address_ip = endpoint.address().to_string();
	network_message.port = endpoint.port();
	m_tcp_node.get().m_recv_queue.push(std::move(network_message));
	// comtinue read
	m_socket.async_read_some(buffer(&m_read_size, sizeof(m_read_size)),
							std::bind(&c_connection::read_size_handler, this, std::placeholders::_1, std::placeholders::_2));
}


void c_connection::delete_me() {
	_dbg_mtx("");
	auto endpoint = m_socket.remote_endpoint();
	std::unique_lock<std::mutex> lg(m_tcp_node.get().m_connection_map_mtx);
	m_tcp_node.get().m_connection_map.erase(endpoint); // remove this object from connection map
	lg.unlock();
	assert(m_myself.unique());
	// delete myself
	m_myself.reset();
}
