#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>

using boost::asio::ip::udp;

enum { max_length = 65535 };

void server(boost::asio::io_service& io_service, unsigned short port)
{
	udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

	size_t received_bytes = 0;
	size_t received_packets = 0;
	auto start_time = std::chrono::steady_clock::now();
	char data[max_length];
	udp::endpoint sender_endpoint;

	for (;;)
	{
		if (!received_packets) {
			start_time = std::chrono::steady_clock::now();
		}
		size_t length = sock.receive_from(
				boost::asio::buffer(data, max_length), sender_endpoint);
		received_packets++;
		received_bytes += length;
		if (received_packets % 100000 == 0) {
			auto time_now = std::chrono::steady_clock::now();
			double sec = ( std::chrono::duration_cast<std::chrono::milliseconds>(time_now - start_time) ).count() / 1000.;
			std::cout << "Receive packages:" << received_packets
				<< ", receive bytes:" << received_bytes
				<< ", speed:" << ((received_bytes * 8) / (1000*1000)) / sec << " Mbits/s" << std::endl;
				//<< ", speed:" << ((static_cast<double>(received_bytes) * 8) / (1000*1000)) / sec << " Mbits/s" << std::endl;

		}

	}
}

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			return 1;
		}

		boost::asio::io_service io_service;

		server(io_service, std::atoi(argv[1]));
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
