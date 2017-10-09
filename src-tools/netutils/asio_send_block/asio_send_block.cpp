#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <boost/asio.hpp>
#include <chrono>
#include <limits>
#include <thread>

using boost::asio::ip::udp;
using std::endl;

enum { max_length = 655035 };

using mysize_t = uint64_t; // size_t is too small to count some things, and overflows, e.g. on r-pi


int main(int argc, char *argv[])
{
	if (argc < 3) {
		std::cout << "Usage: ./client <host> <port> <max_speed(kpck/s)>" << std::endl;
		std::cout << "or ./client <host> <port> <max_speed(kpck/s)> <msg> <msgbytes> <count> " << std::endl;
		return 1;
	}
	const char *host = argv[1];
	const char *port = argv[2];
	const mysize_t speed = std::stoi(argv[3]);
	boost::asio::io_service io_service;

	std::string message;

	bool interactive=true;

	mysize_t bytes, count, request_length;
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

	mysize_t counter_sleep_no=0;
	mysize_t counter_sleep_yes=0;

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
			<< std::string(request, request + std::min(static_cast<mysize_t>(10),request_length))
			<< "\"" << std::endl;
		std::cerr << endl;

		mysize_t burst = 50;

		auto buf = boost::asio::buffer(request, request_length);

		auto time_start = std::chrono::steady_clock::now();

		double target_speed_Mbit_sec = speed; // [MegaBit/sec]
		double target_speed = (target_speed_Mbit_sec/8); // [Byte/microSec]

		mysize_t packets_sent=0;
		mysize_t size_sent = 0;

		while ( ! (packets_sent > count)) {
			for (mysize_t b=0; b<burst; ++b) {
				s.send_to(buf, endpoint);
				++packets_sent;
				size_sent += request_length;
				if (packets_sent > count) break;
			} // burst sending done
			double ellapsed_now =  std::chrono::duration_cast<std::chrono::microseconds>(
				std::chrono::steady_clock::now() - time_start ).count(); // [microSec, since start]
			auto size_next = size_sent + burst * request_length; // [Byte] that much data will be sent after next burst
			auto ellapsed_next = size_next / target_speed; // [microSec]
			auto sleep_next = ellapsed_next - ellapsed_now; // [microSec]
			if (sleep_next > 0) {
				++counter_sleep_yes;
				std::this_thread::sleep_for(std::chrono::microseconds( static_cast<long int>(sleep_next) ));
			}
			else ++counter_sleep_no;

			if ( 0 == ( packets_sent % 100000 ) ) {
				std::cout << "Sent: " << packets_sent << " pck, " << size_sent << " B" << ". "
					<< "Sleep: yes=" << counter_sleep_yes << ", no=" << counter_sleep_no
					<< "\n"
				;
			}
		}
		/*
		double now_recv_ellapsed_sec = ( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time) ).count() / 1000.;
		double now_recv_speed = (i / now_recv_ellapsed_sec) / (1000.); // kpck/s
		if (speed < now_recv_speed) std::this_thread::sleep_for(std::chrono::milliseconds(10));
		if (i % 10000 == 0) std::cerr << "max speed:" <<speed << " kpck/s, current speed:" << now_recv_speed << " kpck/s" << std::endl;
		*/
		if (!interactive) break;
	}
return 0;
}
