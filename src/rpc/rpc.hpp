// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef RPC_HPP
#define RPC_HPP

#include <array>
#include <boost/asio.hpp>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/**
 * @brief The c_rpc_sever class
 * !!! NEVER CHANGE ADDRESS OF THIS CLASS OBJECT !!!
 */
class c_rpc_server final {
	public:
		c_rpc_server(const unsigned short port);
		c_rpc_server(const c_rpc_server &) = delete;
		c_rpc_server & operator = (const c_rpc_server &) = delete;
		c_rpc_server(c_rpc_server &&) = delete;
		c_rpc_server & operator = (c_rpc_server &&) = delete;
		~c_rpc_server();
		/**
		 * @brief add_rpc_function
		 * @param function must be thread safe(will be called from another thread)
		 */
		void add_rpc_function(const std::string &rpc_function_name, std::function<std::string(const std::string &)> &&function);
	private:
		class c_session;
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		std::unique_ptr<std::thread> m_thread_ptr;
		std::mutex m_session_vector_mutex;
		std::list<c_session> m_session_list;
		std::map<std::string, std::function<std::string(const std::string)>> m_rpc_functions_map;

		void accept_handler(const boost::system::error_code &error);
		void remove_session_from_vector(std::list<c_session>::iterator it);

		class c_session {
			public:
				c_session(c_rpc_server *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket);
				c_session(c_session &&) = delete;
				c_session & operator = (c_session && other) = delete;
				void set_iterator_in_session_list(std::list<c_session>::iterator it);
			private:
				std::list<c_session>::iterator m_iterator_in_session_list;
				c_rpc_server *m_rpc_server_ptr;
				boost::asio::ip::tcp::socket m_socket;
				std::string m_received_data;
				std::string m_write_data;
				std::array<uint8_t, 2> m_data_size; // always first 2 bytes of packet == message size

				void read_handler_size(const boost::system::error_code &error, std::size_t bytes_transferred); ///< data readed to m_read_data_size
				void read_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
				void write_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
				void delete_me();
				/**
				 * @brief execute_rpc_command Parse and execute function from m_rpc_functions_map
				 * @param input_message
				 */
				void execute_rpc_command(const std::string &input_message);
		};
};

#endif
