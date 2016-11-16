// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef RPC_HPP
#define RPC_HPP

#include <boost/asio.hpp>
#include <string>
//#include <thread>
#include <vector>

class c_rpc_sever final {
	public:
		c_rpc_sever(const short port);
	private:
		class c_session;
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		std::vector<c_session> m_session_vector;

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
		};
};

#endif
