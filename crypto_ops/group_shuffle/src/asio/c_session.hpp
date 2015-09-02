#ifndef C_SESSION_HPP
#define C_SESSION_HPP

#include <memory>
#include <boost/asio.hpp>
#include "c_net_node.hpp"

#define MAX_RECV_DATA_SIZE 1024

class c_net_node;

class c_session : public std::enable_shared_from_this <c_session> {
	public:
		c_session() = delete;
		c_session(boost::asio::ip::tcp::socket socket, c_net_node *net_node_ptr);
        c_session(boost::asio::ip::tcp::socket socket, std::string m_buffer);
		~c_session();
		void start();
        void do_write();
	private:
		boost::asio::ip::tcp::socket m_socket;
		char m_recv_data[MAX_RECV_DATA_SIZE];
		void do_read();
        const std::string m_buffer;
		c_net_node *m_net_node_ptr; // TODO const
};

#endif // C_SESSION_HPP
