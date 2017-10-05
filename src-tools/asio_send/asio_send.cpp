#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>
#include <chrono>
#include <thread>

using boost::asio::ip::udp;
using std::endl;

enum { max_length = 655035 };

int main(int argc, char *argv[])
{
	if (argc < 3) {
		std::cout << "Usage: ./client <host> <port> <max_speed(kpck/s)>" << std::endl;
		std::cout << "or ./client <host> <port> <max_speed(kpck/s)> <msg> <msgbytes> <count> " << std::endl;
		return 1;
	}
	const char *host = argv[1];
	const char *port = argv[2];
	const size_t speed = std::stoi(argv[3]);
	boost::asio::io_service io_service;

	std::string message;

	bool interactive=true;

	size_t bytes, count, request_length;
	if (argc >= 2+3+1) {
		interactive=false;
		message = argv[4];
		bytes = atoi(argv[5]);
		count = atoi(argv[6]);
	}

	udp::socket s(io_service, udp::endpoint(udp::v4(), 0));

	udp::resolver resolver(io_service);
	udp::endpoint endpoint = *resolver.resolve({udp::v4(), host, port});

	std::cout << endl << "Usage example: give command: " << endl
		<< "  foo 0 1    - this will send text foo (1 time)" << std::endl
		<< "  abc 50 1000   - this will send message of letter 'a' repeated 50 times. This msg will be sent 1000 times."
		<<std::endl;

	std::cout << std::endl;

	while(true) {
		char request[max_length];
		if (interactive) {
			std::cout << "Enter: message bytes count";
			std::cout << std::endl << "-> ";
			std::cin >> message >> bytes >> count;
		}
		if (message.size() > max_length) {
			std::cout << "too long message" << std::endl;
			continue;
		}
		if (bytes == 0) {
			request_length = message.size();
		}
		else {
			request_length = bytes;
			assert(message.size()>=1);
			char ch = message.at(0);
			std::fill_n(request, request_length, ch);
		}

		std::cerr << "Sending now " << count << " time(s) an UDP packet ";
		std::cerr << "datagram: size " << request_length << " B ";
		std::cerr << "data begins with \""
			<< std::string(request, request + std::min(static_cast<size_t>(10),request_length))
			<< "\"" << std::endl;
		std::cerr << endl;
		// auto start_time = std::chrono::steady_clock::now();
		auto buf = boost::asio::buffer(request, request_length);
		for (size_t i=0; i<count; i++) {
			s.send_to(buf, endpoint);
			/*
			double now_recv_ellapsed_sec = ( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time) ).count() / 1000.;
			double now_recv_speed = (i / now_recv_ellapsed_sec) / (1000.); // kpck/s
			if (speed < now_recv_speed) std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (i % 10000 == 0) std::cerr << "max speed:" <<speed << " kpck/s, current speed:" << now_recv_speed << " kpck/s" << std::endl;
			*/
		}
		if (!interactive) break;
	}
return 0;
}
