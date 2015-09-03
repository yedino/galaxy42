#include "c_net_node.hpp"

using namespace boost::asio;

c_net_node::c_net_node () :
	m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v6(), m_port)),
	m_socket(m_io_service),
	m_client_socket(m_io_service),
	m_timer(m_io_service, boost::posix_time::seconds(1))
{
	_info("c_net_node constructor");
	run();
}


void c_net_node::run() {
	_info("run()");
	do_accept();
	for (unsigned int i = 0; i < m_number_of_threads; ++i) {
		m_thread_vector.push_back(std::unique_ptr<std::thread>(new std::thread([this]() { // TODO use make_unique
			m_io_service.run();
		}))); // lambda
	}
}

c_net_node::~c_net_node() {
	_info("c_net_node destructor");
	m_io_service.stop();
	for (auto &thread_ptr : m_thread_vector) {
		thread_ptr->join();
	}
}

void c_net_node::do_accept() {
	//_info("start do_accept");
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code ec) {
		if (!ec) {
			_info("accept OK");
			std::make_shared<c_session>(std::move(m_socket), this)->start();
		}
		else {
			_info("do_accept error: " << ec.message());
		}
		do_accept();
	}); // lambda
	//_info("end do_accept");
}

void c_net_node::add_to_inbox (char *data, size_t size, const ip::address &source_address) {
	_info("add_to_inbox");
	std::lock_guard<std::mutex> lg(m_inbox_mutex);
	s_message message;
	message.m_data.assign(data, size);
	message.m_source_id = source_address.to_string();
	m_inbox.emplace_back(std::move(message));
}


/****************************************/
//c_api_tr
/****************************************/

void c_net_node::write_to_nym (t_nym_id guy, const string &data) {
	_note("start connect to " << guy);
	m_client_socket.async_connect(ip::tcp::endpoint(ip::address::from_string(guy.c_str()), m_port), [this, data](const boost::system::error_code &ec) {
		if(!ec) {
			_note("async_connect handler");
            std::make_shared<c_session>(std::move(m_client_socket), data)->do_write();
		}
		else {
			_note("connect error: " << ec.message());
		}
	}); // lambda
}

vector< s_message > c_net_node::read_or_wait_for_data() {
	std::lock_guard<std::mutex> lg(m_inbox_mutex);
	std::vector<s_message> ret(std::move(m_inbox));
	return ret;
}
