// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef RPC_HPP
#define RPC_HPP

#include "c_tcp_asio_node.hpp"
#include <boost/any.hpp>

/**
 * @brief The c_rpc_server class
 * Wait for RPC command from tcp. Receive message format:
 * command_name;argument1;argument2;argument3 ...
 */
class c_rpc_server final {
	private:
		std::unique_ptr<c_connection_base> m_connection_node;
		std::atomic<bool> m_stop_flag; // TODO atomic_falg ?
		std::unique_ptr<std::thread> m_work_thread;
		void main_loop(); ///< loop run in m_work_thread
		/**
		 * @brief m_command_map
		 * Functions stored in this map will be invoked in m_work_thread and should be thread safe
		 * command name => function
		 */
		std::map<std::string, std::function<bool(const std::string &)>> m_command_map;
		std::mutex m_command_map_mtx;
	public:
		c_rpc_server(const unsigned int port);
		void register_function(const std::string &command_name, std::function<bool(const std::string &)> function);
		~c_rpc_server();

};

void send_tcp_msg(const std::string &msg, const std::string addr = "127.0.0.1", int port = 9040);
void rpc_demo();
bool rpc_example_function(const std::string &arguments);


#endif // RPC_HPP
