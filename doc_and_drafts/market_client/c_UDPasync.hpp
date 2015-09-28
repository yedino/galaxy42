#ifndef C_UDPASYNC_HPP
#define C_UDPASYNC_HPP

#include "c_statistics.hpp"
#include "c_locked_queue.hpp"

#include "c_connect.hpp"

#include <array>
#include <thread>

static const int NetworkBufferSize = 4096;

using boost::asio::ip::udp;

class c_UDPasync {
  public:
	c_UDPasync(std::string host, std::string server_port, unsigned short local_port = 0);
	~c_UDPasync();

	void Send(std::string message);

	inline bool HasMessages() {
		return !incomingMessages.empty();
	};

	inline std::string PopMessage() {
		if (incomingMessages.empty()) throw std::logic_error("No messages to pop");
		return incomingMessages.pop();
	};

  private:
	// Network send/receive stuff
	boost::asio::io_service io_service;
	udp::socket socket;
	udp::endpoint server_endpoint;
	udp::endpoint remote_endpoint;
	std::array<char, NetworkBufferSize> recv_buffer;
	std::thread service_thread;

	// Queues for messages
	c_locked_queue<std::string> incomingMessages;

	void start_receive();
	void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
	void run_service();

	c_UDPasync(c_UDPasync&) = delete;

	// c_statistics
	c_statistics statistics;
};
#endif // C_UDPASYNC_HPP

