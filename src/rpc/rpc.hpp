// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef RPC_HPP
#define RPC_HPP

#include <array>
#include <boost/asio.hpp>
#include <functional>
#include <list>
#include <map>
#include "../mutex.hpp"
#include <string>
#include <thread>
#include <vector>
#include <sodium.h> // for sha512 key size e.g. crypto_auth_hmacsha512_KEYBYTES
#include <xint.hpp>
#include <json.hpp>

/**
 * @brief The c_rpc_sever class
 * !!! NEVER CHANGE ADDRESS OF THIS CLASS OBJECT !!!
 * network message format:
 * n[2 octets] == size of message
 * message[n octets] == message in json format
 * crypto_hash_sha512_BYTES bytes of HMAC SHA-512
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
		void add_rpc_function(const std::string &rpc_function_name, std::function<nlohmann::json(const std::string &)> &&function);
	private:
		class c_session;
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::acceptor m_acceptor;
		boost::asio::ip::tcp::socket m_socket;
		std::thread m_thread;
		Mutex m_session_vector_mutex;
		std::list<c_session> m_session_list;
		Mutex m_rpc_functions_map_mutex;
		std::map<std::string, std::function<nlohmann::json(const std::string)>> m_rpc_functions_map GUARDED_BY(m_rpc_functions_map_mutex);
		std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> m_hmac_key; ///< for hmac authentication key, shold be loaded from conf file TODO
		xint m_session_counter = 0;

		void accept_handler(const boost::system::error_code &error);
		void remove_session_from_vector(std::list<c_session>::iterator it);

		class c_session {
			public:
				c_session(c_rpc_server *rpc_server_ptr,
				                boost::asio::ip::tcp::socket &&socket,
				                const std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> &hmac_key,
				                xint session_id);
				c_session(c_session &&) = delete;
				c_session & operator = (c_session && other) = delete;
				void set_iterator_in_session_list(std::list<c_session>::iterator it);
			private:
				std::list<c_session>::iterator m_iterator_in_session_list; // needed for delete_me()
				c_rpc_server *m_rpc_server_ptr; // needed for delete_me()
				boost::asio::ip::tcp::socket m_socket;
				std::string m_received_data;
				std::string m_write_data;
				std::array<unsigned char, 2> m_data_size; // always first 2 bytes of packet == message size
				std::array<unsigned char, crypto_auth_hmacsha512_BYTES> m_hmac_authenticator;
				std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> m_hmac_key; ///< for hmac authentication key
				const xint m_session_id;
				xint m_rpc_command_counter = 499; // next command will have number 500

				std::string get_command_id();
				void read_handler_size(const boost::system::error_code &error, std::size_t bytes_transferred); ///< data readed to m_read_data_size
				void read_handler_data(const boost::system::error_code &error, std::size_t bytes_transferred);
				void read_handler_hmac(const boost::system::error_code &error, std::size_t bytes_transferred);
				void write_handler(const boost::system::error_code &error, std::size_t bytes_transferred);
				/**
				 * @brief delete_me
				 * remove this object from c_rpc_server::m_session_list
				 */
				void delete_me();
				/**
				 * @brief execute_rpc_command Parse and execute function from m_rpc_functions_map
				 * @param input_message
				 */
				void execute_rpc_command(const std::string &input_message);
		};
};

#endif
