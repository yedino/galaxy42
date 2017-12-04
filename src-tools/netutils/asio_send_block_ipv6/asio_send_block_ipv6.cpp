#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdio>
#include <vector>

using namespace boost::asio;

int main( int size, char** args) {

        unsigned int packet_size = 0;

	if(size == 2) {
	        sscanf(args[1], "%d",&packet_size);
	}else{
	        packet_size =1500;
	}
	if(packet_size>65535) {
	        std::cout<<"packet is to big"<<"\n";
		return 0;
	}

	std::cout<<"packet size: "<<packet_size<<"\n";

		boost::asio::io_service io_service;
		ip::udp::socket s(io_service,  ip::udp::endpoint(ip::address_v6(), 19000));
		ip::udp::socket s2(io_service,  ip::udp::endpoint(ip::address_v6(), 19001));
		std::vector<unsigned char> buff;		//tu wielkosci 500 ,1000 ,2000 ,5000 10000, 65000
		buff.resize(packet_size);
		//		buff.fill(0);

	auto thread_lambda = [&] {
		while(1) {
			s.send_to(boost::asio::buffer(buff), ip::udp::endpoint(ip::address_v6::from_string("fd11:1111:1111:1111:1111:1111:1111:1112"), 9000));
		}
	}; // lambda
	auto thread_lambda2 = [&] {
		while(1) {
			s2.send_to(boost::asio::buffer(buff), ip::udp::endpoint(ip::address_v6::from_string("fd11:1111:1111:1111:1111:1111:1111:1112"), 9001));
		}
	}; // lambda
	std::vector<std::thread> threads;
	threads.emplace_back(thread_lambda);
	threads.emplace_back(thread_lambda2);

	for (auto &thread : threads)
		thread.join();
}
