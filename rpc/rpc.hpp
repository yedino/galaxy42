#ifndef RPC_HPP
#define RPC_HPP

#include "c_tcp_asio_node.hpp"


class c_rpc_server final {
	private:
		std::unique_ptr<c_connection_base> m_connection_node;
		std::atomic<bool> m_stop_flag; // TODO atomic_falg ?
		std::unique_ptr<std::thread> m_work_thread;
		void main_loop(); ///< loop run in thread
	public:
		c_rpc_server(const unsigned int port = 42000);
		~c_rpc_server();


};

void send_tcp_msg(const std::string &msg, const std::string addr = "127.0.0.1", int port = 9040);
void rpc_demo();


#endif // RPC_HPP
