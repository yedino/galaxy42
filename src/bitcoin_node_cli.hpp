#ifndef BITCOIN_NODE_CLI_HPP
#define BITCOIN_NODE_CLI_HPP

#include <libs0.hpp>

class bitcoin_node_cli final {
	public:
		bitcoin_node_cli(const std::string &ip_address, unsigned short port);
		uint32_t get_balance();
	private:
		boost::asio::io_service m_io_service;
		const boost::asio::ip::tcp::endpoint m_btc_rpc_endpoint;
		boost::asio::ip::tcp::socket m_socket;
};

#endif // BITCOIN_NODE_CLI_HPP
