// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "rpc.hpp"
#include "../trivialserialize.hpp"
#include <json.hpp>
#include <sodium.h>

#include <libs0.hpp>

#define dbg(X) _info("RPC: " << X);
//#define _dbg(X) std::cout << "RPC: " << X << std::endl;

c_rpc_server::c_rpc_server(const unsigned short port)
:
	m_io_service(),
	m_acceptor(m_io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), port)),
	m_socket(m_io_service),
	m_thread(),
	m_hmac_key()
{
	m_hmac_key.fill(0x42); // TODO load this from conf file
	// start waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
	dbg("Starting RPC server thread");
	m_thread = std::thread([this]() {
		dbg("RPC thread start");
		try {
			boost::system::error_code ec;
			dbg("io_service run");
			m_io_service.run(ec);
			dbg("end of io_service run");
			if (ec)
				dbg("error code " << ec.message());
			dbg("io_service reset");
			m_io_service.reset();
		} catch (const std::exception &e) {
			dbg("io_service exception" << e.what());
		} catch (...) {
			dbg("catch unhandled exception");
		}

		dbg("RPC thread stop");
	}); // lambda
}

c_rpc_server::~c_rpc_server() {
	dbg("rpc server destructor");
	m_io_service.stop();
	dbg("io service stopped, join thread");
	m_thread.join();
	dbg("thread joined");
}

void c_rpc_server::add_rpc_function(const std::string &rpc_function_name, std::function<nlohmann::json (const std::string&)> &&function) {
	LockGuard<Mutex> lg(m_rpc_functions_map_mutex);
	m_rpc_functions_map.emplace(rpc_function_name, std::move(function));
}

void c_rpc_server::accept_handler(const boost::system::error_code &error) {
	dbg("Connected");
	if (!error) {
		LockGuard<Mutex> lg(m_session_vector_mutex);
		m_session_list.emplace_back(this, std::move(m_socket), m_hmac_key, ++m_session_counter);
		m_session_list.back().set_iterator_in_session_list(--m_session_list.end()); // set internal interator to me
	}
	// continue waiting for new connection
	m_acceptor.async_accept(m_socket, [this](boost::system::error_code error) {
		accept_handler(error);
	});
}

void c_rpc_server::remove_session_from_vector(std::list<c_session>::iterator it) {
	LockGuard<Mutex> lg(m_session_vector_mutex);
	m_session_list.erase(it);
}

/*************************************************************************************/

c_rpc_server::c_session::c_session(c_rpc_server *rpc_server_ptr,
                                   boost::asio::ip::tcp::socket &&socket,
                                   const std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> &hmac_key,
                                   xint session_id)
:
	m_rpc_server_ptr(rpc_server_ptr),
	m_socket(std::move(socket)),
	m_received_data(),
	m_write_data(),
	m_hmac_key(hmac_key),
	m_session_id(session_id)
{
	// start reading size (2 bytes)
	async_read(m_socket, boost::asio::buffer(&m_data_size.at(0), m_data_size.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler_size(error, bytes_transferred);
	});
}

void c_rpc_server::c_session::read_handler_size(const boost::system::error_code &error, std::size_t bytes_transferred) {
	dbg("read handler size");
	if (error) {
		dbg("asio error " << error.message());
		delete_me();
		return;
	}
	if (bytes_transferred != 2) {
		dbg("bytes_transferred != 2");
		delete_me();
		return;
	}
	uint16_t message_size = static_cast<uint16_t>(m_data_size.at(0) << 8);
	message_size += m_data_size.at(1);
	m_received_data.resize(message_size, 0); // prepare buffer for message
	dbg("message size = " << message_size);
	async_read(m_socket, boost::asio::buffer(&m_received_data.at(0), m_received_data.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler_data(error, bytes_transferred);
	});
}

void c_rpc_server::c_session::read_handler_data(const boost::system::error_code &error, std::size_t bytes_transferred) {
	dbg("readed " << bytes_transferred << " bytes of data");
	if (error) {
		dbg("asio error " << error.message());
		delete_me();
		return;
	}
	// parsing message
	dbg("received message " << m_received_data);
	async_read(m_socket, boost::asio::buffer(&m_hmac_authenticator.at(0), m_hmac_authenticator.size()),
		[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
			read_handler_hmac(error, bytes_transferred);
	});
}

void c_rpc_server::c_session::read_handler_hmac(const boost::system::error_code &error, std::size_t bytes_transferred) {
	dbg("readed " << bytes_transferred << " bytes of hmac authenticator");
	dbg("authenticate message");
	if (error) {
		dbg("asio error " << error.message());
		delete_me();
		return;
	}
	int ret = crypto_auth_hmacsha512_verify(m_hmac_authenticator.data(),
	                                        reinterpret_cast<const unsigned char *>(m_received_data.data()),
	                                        m_received_data.size(),
	                                        m_hmac_key.data()
	);
	if (ret == -1) {
		_warn("hmac authentication error");
		delete_me();
		return;
	}
	assert(ret == 0);
	try {
		execute_rpc_command(m_received_data);
	} catch (const std::exception &e) {
		_erro( "exception read_handler " << e.what());
		_erro( "close connection\n" );
		delete_me();
		return;
	}
}

void c_rpc_server::c_session::write_handler(const boost::system::error_code &error, std::size_t bytes_transferred) {
	UNUSED(bytes_transferred);
	try {
		if (error) {
			dbg("asio error " << error.message());
			delete_me();
			return;
		}
		// continue reading
		async_read(m_socket, boost::asio::buffer(&m_data_size.at(0), m_data_size.size()),
			[this](const boost::system::error_code &error, std::size_t bytes_transferred) {
				read_handler_size(error, bytes_transferred);
		});
	}
	catch (const std::exception &e) {
		_erro( "exception in write_handler " << e.what() );
		_erro( "close connection\n" );
		delete_me();
		return;
	}
}

void c_rpc_server::c_session::set_iterator_in_session_list(std::list<c_session>::iterator it) {
	m_iterator_in_session_list = it;
}

std::string c_rpc_server::c_session::get_command_id()
{
	std::stringstream sstream;
	sstream << m_session_id << "-srv" << ++m_rpc_command_counter;
	return sstream.str();
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
                const std::string cmd_name = j["cmd"];//.begin().value();
                dbg("cmd name " << cmd_name);
		// calling rpc function
		nlohmann::json json_response;
		{
			LockGuard<Mutex> lg(m_rpc_server_ptr->m_rpc_functions_map_mutex);
			json_response = m_rpc_server_ptr->m_rpc_functions_map.at(cmd_name)(input_message);
		}
		json_response["id"] = get_command_id();

		if(j.find("id")!= j.end()) {
		        json_response["re"] = j["id"];
		}

		const std::string response = json_response.dump();

		// serialize response
		assert(response.size() <= std::numeric_limits<uint16_t>::max());
		uint16_t size = static_cast<uint16_t>(response.size());
		m_write_data.resize(size + 2); ///< 2 first bytes for size
		m_write_data.at(0) = static_cast<char>(size >> 8);
		m_write_data.at(1) = static_cast<char>(size & 0xFF);
		for (size_t i = 0; i < response.size(); ++i)
			m_write_data.at(i + 2) = response.at(i);
		// send response

		dbg("send packet");
		dbg(m_write_data);
		std::array<unsigned char, crypto_auth_hmacsha512_BYTES> hash;
		int ret = crypto_auth_hmacsha512(hash.data(), reinterpret_cast<unsigned char *>(&m_write_data.at(2)), size, m_rpc_server_ptr->m_hmac_key.data());
		if (ret != 0) _throw_error(std::runtime_error("crypto_auth_hmacsha512 error"));
		dbg("hmac");
		//for (const auto & byte : hash) std::cout << std::hex << "0x" << static_cast<int>(byte) << " ";
		//std::cout << std::dec << std::endl;
		std::array<boost::asio::const_buffer, 2> buffers = {
			{boost::asio::buffer(m_write_data.data(), m_write_data.size()),
			boost::asio::buffer(hash.data(), hash.size())}
		};
		m_socket.async_write_some(buffers,
			[this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				write_handler(error, bytes_transferred);
		});
	}
	catch (const std::exception &e) {
		_erro( "exception in execute_rpc_command " << e.what() );
		_erro( "close connection\n" );
		delete_me();
		return;
	}
}
