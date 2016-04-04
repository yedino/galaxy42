#ifndef C_CONNECTION_BASE_H
#define C_CONNECTION_BASE_H

#include <string>

/**
 * contains raw data and source/destination address
 */
class c_network_message {
	public:
		c_network_message();
		c_network_message(c_network_message &&) = default;
		std::string address_ip;
		unsigned short port;
		std::string data;
};

class c_connection_base {
	public:
		/**
		 * sends message and always returns immediately
		 * @param message will be consumed
		 * Exception safety: Strong exception guarantee
		 */
		virtual void send(c_network_message && message) = 0;

		/**
		 * The function call always returns immediately
		 * @returns empty message if error or no data received
		 * Exception safety: Strong exception guarantee
		 */
		virtual c_network_message receive() = 0;

		virtual ~c_connection_base() = default;
};

#endif // C_CONNECTION_BASE_H
