// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "rpc.hpp"
#include "../trivialserialize.hpp"
#include <json/json.h>

c_rpc_server::c_rpc_server(const short port)
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

void c_rpc_server::add_rpc_function(const std::string &rpc_function_name, std::function<std::string (const std::string&)> &&function) {
	m_rpc_functions_map.emplace(rpc_function_name, std::move(function)); // TODO forward arguments
}

void c_rpc_server::accept_handler(const boost::system::error_code &error) {
	if (!error) {
		m_session_vector.emplace_back(m_session_vector.size(), this, std::move(m_socket));
	}
	// continue waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
}

void c_rpc_server::remove_session_from_vector(const size_t index) {
	if (index >= m_session_vector.size()) throw std::out_of_range("Not found session with index " + std::to_string(index));
	m_session_vector.erase(m_session_vector.begin() + index);
}


c_rpc_server::c_session::c_session(size_t index_in_session_vector, c_rpc_server *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket)
:
	m_index_in_session_vector(index_in_session_vector),
	m_rpc_server_ptr(rpc_server_ptr),
	m_socket(std::move(socket)),
	m_received_data(1024, 0), // fill
	m_write_data()
{
	// start reading
	m_socket.async_read_some(boost::asio::buffer(&m_received_data[0], m_received_data.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler(error, bytes_transferred);
	});
}

c_rpc_server::c_session &c_rpc_server::c_session::operator =(c_rpc_server::c_session && other) noexcept {
	m_index_in_session_vector = other.m_index_in_session_vector;
	m_rpc_server_ptr = other.m_rpc_server_ptr;
	m_socket = std::move(other.m_socket);
	m_received_data = std::move(other.m_received_data);
}

void c_rpc_server::c_session::read_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	if (error) {
		delete_me();
		return;
	}
	// parsing message
	trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), m_received_data);
	if (parser.pop_integer_u<1, uint8_t>() != 0xFF) {
		delete_me();
		return;
	}
	uint64_t message_size = parser.pop_integer_uvarint();
	std::string message = parser.pop_varstring();
	if (message.size() != message_size) { // TODO message can be chunked ?
		delete_me();
		return;
	}
	execute_rpc_command(message);
}

void c_rpc_server::c_session::write_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	if (error) {
		delete_me();
		return;
	}
	// continue reading
	m_socket.async_read_some(boost::asio::buffer(&m_received_data[0], m_received_data.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler(error, bytes_transferred);
	});
}

void c_rpc_server::c_session::delete_me() {
	m_rpc_server_ptr->remove_session_from_vector(m_index_in_session_vector);
}

void c_rpc_server::c_session::execute_rpc_command(const std::string &input_message) {
	try {
		Json::Value json_root(input_message);
		const std::string cmd_name = json_root.get("cmd", "UTF-8").asString();
		// calling rpc function
		const std::string response = m_rpc_server_ptr->m_rpc_functions_map.at(cmd_name)(input_message);
		// serialize response
		trivialserialize::generator generator(100);
		generator.push_integer_u<1, uint8_t>(0xFF);
		generator.push_integer_uvarint(response.size());
		generator.push_varstring(response);
		m_write_data = std::move(generator.str_move());
		// send response
		m_socket.async_write_some(boost::asio::buffer(m_write_data.data(), m_write_data.size()),
			[this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				write_handler(error, bytes_transferred);
		});
	}
	catch (...) {
		delete_me();
	}

}
