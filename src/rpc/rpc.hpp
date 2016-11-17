// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef RPC_HPP
#define RPC_HPP

#include <boost/asio.hpp>
#include <functional>
#include <map>
#include <string>
//#include <thread>
#include <vector>

/**
 * @brief The c_rpc_sever class
 * !!! NEVER CHANGE ADDRESS OF THIS CLASS OBJECT !!!
 */
class c_rpc_sever final {
	public:
		c_rpc_sever(const short port);
		/**
		 * @brief add_rpc_function
		 * @param function must be thread safe(will be called from another thread)
		 */
		void add_rpc_function(const std::string &rpc_function_name, std::function<std::string(const std::vector<std::string>)> &&function);
	private:
		class c_session;
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		std::vector<c_session> m_session_vector;
		std::map<std::string, std::function<std::string(const std::vector<std::string>)>> m_rpc_functions_map;

		void accept_handler(const boost::system::error_code &error);
		void remove_session_from_vector(const size_t index);

		class c_session {
			public:
				c_session(size_t index_in_session_vector, c_rpc_sever *rpc_server_ptr, boost::asio::ip::tcp::socket &&socket);
			private:
				size_t m_index_in_session_vector; // my index in m_session_vector
				c_rpc_sever *m_rpc_server_ptr;
				boost::asio::ip::tcp::socket m_socket;
				std::string m_received_data;

				void read_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
				void delete_me();
		};
};

#endif
