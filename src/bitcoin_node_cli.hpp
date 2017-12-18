#ifndef BITCOIN_NODE_CLI_HPP
#define BITCOIN_NODE_CLI_HPP

#include <libs0.hpp>
#include <boost/asio.hpp>
#include <curl/curl.h>

class c_curl_ptr final {
	public:
		c_curl_ptr();
		~c_curl_ptr();
		CURL *get_raw_ptr() const;
	private:
		CURL *m_ptr;
};

//////////////////////////////////////////////////////

class bitcoin_node_cli final {
	public:
		bitcoin_node_cli(const std::string &ip_address="127.0.0.1", unsigned short port=18332);
		/**
		 * @brief get_balance this funstion is thread safe
		 * @return amount of BTC (in satoshi)
		 */
		uint32_t get_balance() const;

		/**
		 * @brief get_new_address
		 * @return new bitcoin address
		 */
		std::string get_new_address() const;
	private:
		mutable boost::asio::io_service m_io_service;
		const boost::asio::ip::tcp::endpoint m_btc_rpc_endpoint;
		std::string generate_request_prototype() const;
		std::string send_request_and_get_response(const std::string &request) const;
		void remove_POST_data(std::string &data_with_POST) const;
};

#endif // BITCOIN_NODE_CLI_HPP
