// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "rpc.hpp"
#include "../trivialserialize.hpp"
#include "../json/json.hpp"

#define _dbg(X) do std::cout << X << "\n"; while(0)

c_rpc_server::c_rpc_server(const short port)
:
	m_io_service(),
	m_acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
	m_socket(m_io_service),
	m_thread_ptr(nullptr)
{
	// start waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
	m_thread_ptr = make_unique<std::thread>([this](){m_io_service.run();});
}

c_rpc_server::~c_rpc_server() {
	m_io_service.stop();
	m_thread_ptr->join();
}

void c_rpc_server::add_rpc_function(const std::string &rpc_function_name, std::function<std::string (const std::string&)> &&function) {
	m_rpc_functions_map.emplace(rpc_function_name, std::move(function)); // TODO forward arguments
}

void c_rpc_server::accept_handler(const boost::system::error_code &error) {
	_dbg("Connected");
	if (!error) {
		m_session_list.emplace_back(this, std::move(m_socket));
		m_session_list.back().set_iterator_in_session_list(--m_session_list.end());
	}
	// continue waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
}

void c_rpc_server::remove_session_from_vector(std::list<c_session>::iterator it) {
	std::lock_guard<std::mutex> lg(m_session_vector_mutex);
//	if (index >= m_session_vector.size()) throw std::out_of_range("Not found session with index " + std::to_string(index));
	m_session_list.erase(it);
	// update indexes in sessions
	//for (size_t i = 0; i < m_session_vector.size(); ++i)
//		m_session_vector[i].set_index_in_session_vector(i);
}


c_rpc_server::c_session::c_session(c_rpc_server *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket)
:
//	m_iterator_in_session_list(iterator_in_session_list),
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

c_rpc_server::c_session &c_rpc_server::c_session::operator =(c_rpc_server::c_session && other) {
	m_index_in_session_vector = other.m_index_in_session_vector;
	m_rpc_server_ptr = other.m_rpc_server_ptr;
	m_socket = std::move(other.m_socket);
	m_received_data = std::move(other.m_received_data);
}

void c_rpc_server::c_session::read_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	_dbg("readed " << bytes_transferred << " bytes");
	try {
		if (error) {
			_dbg("asio error " << error.message());
			delete_me();
			return;
		}
		// parsing message
		trivialserialize::parser parser(trivialserialize::parser::tag_caller_must_keep_this_string_valid(), m_received_data);
		uint64_t message_size = parser.pop_integer_uvarint();
		std::string message = parser.pop_varstring();
		if (message.size() != message_size) { // TODO message can be chunked ?
			delete_me();
			return;
		}
		_dbg("received message " << message);
		execute_rpc_command(message);
	}
	catch (const std::exception &e) {
		std::cerr << "exception read_handler " << e.what() << "\n";
		std::cerr << "close connection\n";
		delete_me();
		return;
	}
}

void c_rpc_server::c_session::write_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	try {
		if (error) {
			_dbg("asio error " << error.message());
			delete_me();
			return;
		}
		// continue reading
		m_socket.async_read_some(boost::asio::buffer(&m_received_data[0], m_received_data.size()),
			[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
				read_handler(error, bytes_transferred);
		});
	}
	catch (const std::exception &e) {
		std::cerr << "exception in write_handler " << e.what() << "\n";
		std::cerr << "close connection\n";
		delete_me();
		return;
	}
}

void c_rpc_server::c_session::set_iterator_in_session_list(std::list<c_session>::iterator it) {
	m_iterator_in_session_list = it;
}

void c_rpc_server::c_session::delete_me() {
	if (m_socket.is_open()) {
		m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
		m_socket.cancel();
		m_socket.close();
	}
	m_rpc_server_ptr->remove_session_from_vector(m_iterator_in_session_list);
}

void c_rpc_server::c_session::execute_rpc_command(const std::string &input_message) {
	try {
		nlohmann::json j = nlohmann::json::parse(input_message);
		const std::string cmd_name = j.begin().value();
		_dbg("cmd name " << cmd_name);
		// calling rpc function
		const std::string response = m_rpc_server_ptr->m_rpc_functions_map.at(cmd_name)(input_message);
		// serialize response
		trivialserialize::generator generator(100);
		generator.push_integer_uvarint(response.size());
		generator.push_varstring(response);
		m_write_data = std::move(generator.str_move());
		// send response
		m_socket.async_write_some(boost::asio::buffer(m_write_data.data(), m_write_data.size()),
			[this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				write_handler(error, bytes_transferred);
		});
	}
	catch (const std::exception &e) {
		std::cerr << "exception in execute_rpc_command " << e.what() << "\n";
		std::cerr << "close connection\n";
		delete_me();
		return;
	}
}
