// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "rpc.hpp"
#include "../trivialserialize.hpp"
#include <json.hpp>

#define _dbg(X) do std::cout << X << "\n"; while(0)

c_rpc_server::c_rpc_server(const unsigned short port)
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
	_dbg("Starting RPC server thread");
	m_thread_ptr = make_unique<std::thread>([this]() {
		_dbg("RPC thread start");
		m_io_service.run();
		_dbg("RPC thread stop");
	});
	if (!m_thread_ptr)
		throw std::runtime_error("Can not start rpc server thread");
}

c_rpc_server::~c_rpc_server() {
	_dbg("rpc server destructor");
	m_io_service.stop();
	_dbg("io service stopped, join thread");
	if (m_thread_ptr) {
		m_thread_ptr->join();
		_dbg("thread joined");
	} else {
		_dbg("thread pointer == nullptr, thread not joined");
	}
}

void c_rpc_server::add_rpc_function(const std::string &rpc_function_name, std::function<std::string (const std::string&)> &&function) {
	m_rpc_functions_map.emplace(rpc_function_name, std::move(function)); // TODO forward arguments
}

void c_rpc_server::accept_handler(const boost::system::error_code &error) {
	_dbg("Connected");
	if (!error) {
		std::lock_guard<std::mutex> lg(m_session_vector_mutex);
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
	m_session_list.erase(it);
}


c_rpc_server::c_session::c_session(c_rpc_server *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket)
:
	m_rpc_server_ptr(rpc_server_ptr),
	m_socket(std::move(socket)),
	m_received_data(1024, 0), // fill
	m_write_data()
{
	// start reading size (2 bytes)
	async_read(m_socket, boost::asio::buffer(&m_data_size[0], m_data_size.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler_size(error, bytes_transferred);
	});
}

void c_rpc_server::c_session::read_handler_size(const boost::system::error_code &error, std::size_t bytes_transferred) {
	_dbg("read handler size");
	if (error) {
		_dbg("asio error " << error.message());
		delete_me();
		return;
	}
	if (bytes_transferred != 2) {
		_dbg("bytes_transferred != 2");
		delete_me();
		return;
	}
	uint16_t message_size = static_cast<uint16_t>(m_data_size[0] << 8);
	message_size += m_data_size[1];
	m_received_data.resize(message_size, 0); // prepare buffer for message
	async_read(m_socket, boost::asio::buffer(&m_received_data[0], m_received_data.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler(error, bytes_transferred);
	});
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
		_dbg("received message " << m_received_data);
		execute_rpc_command(m_received_data);
	}
	catch (const std::exception &e) {
		std::cerr << "exception read_handler " << e.what() << "\n";
		std::cerr << "close connection\n";
		delete_me();
		return;
	}
}

void c_rpc_server::c_session::write_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	UNUSED(bytes_transferred);
	try {
		if (error) {
			_dbg("asio error " << error.message());
			delete_me();
			return;
		}
		// continue reading
		async_read(m_socket, boost::asio::buffer(&m_data_size[0], m_data_size.size()),
			[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
				read_handler_size(error, bytes_transferred);
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
		assert(response.size() <= std::numeric_limits<uint16_t>::max());
		uint16_t size = static_cast<uint16_t>(response.size());
		m_write_data.resize(size + 2); ///< 2 first bytes for size
		m_write_data[0] = static_cast<char>(size >> 8);
		m_write_data[1] = static_cast<char>(size & 0xFF);
		for (size_t i = 0; i < response.size(); ++i)
			m_write_data[i + 2] = response[i];
		// send response

		_dbg("send packet");
		_dbg(m_write_data);
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
