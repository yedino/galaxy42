// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "rpc.hpp"

c_rpc_sever::c_rpc_sever(const short port)
:
	m_io_service(),
	m_acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	m_socket(m_io_service)
{
	// start waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
}

void c_rpc_sever::accept_handler(const boost::system::error_code &error) {
	if (!error) {
		m_session_vector.emplace_back(m_session_vector.size(), this, std::move(m_socket));
	}
	// continue waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
}


c_rpc_sever::c_session::c_session(size_t index_in_session_vector, const c_rpc_sever *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket)
:
	m_index_in_session_vector(index_in_session_vector),
	m_rpc_server_ptr(rpc_server_ptr),
	m_socket(std::move(socket)),
	m_received_data(1024, 0) // fill
{
	m_socket.async_read_some(boost::asio::buffer(&m_received_data[0], m_received_data.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler(error, bytes_transferred);
	});
}

void c_rpc_sever::c_session::read_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {

}
