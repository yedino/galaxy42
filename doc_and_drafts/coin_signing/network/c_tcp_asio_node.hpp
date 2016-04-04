#ifndef C_TCP_ASIO_NODE_H
#define C_TCP_ASIO_NODE_H

#include "c_connection_base.hpp"
#include <boost/asio.hpp>

class c_tcp_asio_node : public c_connection_base
{
	public:
		void send(c_network_message && message) override;
		c_network_message receive() override;

};

#endif // C_TCP_ASIO_NODE_H
