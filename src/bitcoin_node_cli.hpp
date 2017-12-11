#ifndef BITCOIN_NODE_CLI_HPP
#define BITCOIN_NODE_CLI_HPP

#include <libs0.hpp>

class bitcoin_node_cli final {
	public:
		bitcoin_node_cli(const std::string &ip_address="127.0.0.1", unsigned short port=18332);
		/**
		 * @brief get_balance this funstion is thread safe
		 * @return amount of BTC (in satoshi)
		 */
		uint32_t get_balance() const;
	private:
		mutable boost::asio::io_service m_io_service;
		const boost::asio::ip::tcp::endpoint m_btc_rpc_endpoint;
		std::string generate_request_prototype() const;
		std::string send_request_and_get_response(const std::string &request) const;
};

#endif // BITCOIN_NODE_CLI_HPP