#include "c_session.hpp"

using namespace boost::asio;

c_session::c_session (boost::asio::ip::tcp::socket socket, c_net_node *net_node_ptr) :
	m_socket(std::move(socket)),
	m_net_node_ptr(net_node_ptr)
{
}

c_session::c_session (boost::asio::ip::tcp::socket socket, std::string m_buffer) :
    m_socket(std::move(socket)),
    m_buffer(std::move(m_buffer))
{
}

c_session::~c_session() {
	_info("c_session destructor");
}


void c_session::start() {
	do_read();
}

void c_session::do_read() {
	_info("start do read");
	auto self(shared_from_this());
	m_socket.async_read_some(buffer(m_recv_data, MAX_RECV_DATA_SIZE), [this, self](boost::system::error_code ec, std::size_t length) {
		if (!ec) {
			_info("do read data");
			m_net_node_ptr->add_to_inbox(m_recv_data, length, m_socket.remote_endpoint().address());
			//do_read();
			_info("read data size: " << length);
		}
		else {
			_info("do read error " << ec.message());
		}
	}); // lambda
	//_info("end do read");
}

void c_session::do_write () {
	_note("start do_write");
	auto self(shared_from_this());
    m_socket.async_write_some(buffer(m_buffer.data(), m_buffer.size()),
		[this, self] (boost::system::error_code ec, std::size_t length) {
			if(!ec) {
				_note("data write OK");
				_note("length = " << length);
				//_note("data write size " << data.size());
			}
			else {
				_note("data write error: " << ec.message());
			}
		}
	);
}

void c_session::do_write (const string &data) {
	_note("start do_write(data)");
	//auto self(shared_from_this());
    m_socket.async_write_some(buffer(data.data(), data.size()),
		[this] (boost::system::error_code ec, std::size_t length) { // XXX [this, self]
			if(!ec) {
				_note("data write OK");
				_note("length = " << length);
				//_note("data write size " << data.size());
			}
			else {
				_note("data write error: " << ec.message());
			}
		}
	);	
}


