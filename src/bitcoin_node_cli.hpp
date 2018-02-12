#ifndef BITCOIN_NODE_CLI_HPP
#define BITCOIN_NODE_CLI_HPP

#include <libs0.hpp>
#include <boost/asio.hpp>
#include <curl/curl.h>
#include <mutex>

class c_curl_ptr final {
	public:
		c_curl_ptr();
		~c_curl_ptr();
		CURL *get_raw_ptr() const;

		static std::once_flag s_curl_init_flag;

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
		const std::string m_rpc_http_address; // i.e. http://127.0.0.1:18332 for localhost testnet
		std::string send_request_and_get_response(const std::string &request) const;
		static size_t write_cb (void *ptr, size_t size, size_t nmemb, std::string *str);
};

#endif // BITCOIN_NODE_CLI_HPP
