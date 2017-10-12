#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <mutex>

#include "../../tools_helper.hpp"

using boost::asio::ip::udp;

using t_counter = uint64_t;

constexpr size_t max_length = 65535;
constexpr size_t threads_count = 2;

char data[threads_count][max_length];
c_timerfoo g_timer(20);

void handler(const boost::system::error_code &ec, size_t length, udp::socket &sock, char *data, std::mutex &sock_mutex)
{
	std::cout << "get packet sizze:" << length << std::endl;
	g_timer.add(1, length);

	udp::endpoint sender_endpoint;

	std::lock_guard<std::mutex> lg(sock_mutex);
	sock.async_receive_from(
			boost::asio::buffer(data, max_length), sender_endpoint
			, [&sock, &data, &sock_mutex](const boost::system::error_code &ec, size_t length){handler(ec, length, sock, data, sock_mutex);} );
}

double run_tests(boost::asio::io_service& io_service, unsigned short port, size_t threads_count)
{
	//udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
	udp::socket sock(io_service, udp::endpoint(boost::asio::ip::address_v4::any(), port));

	std::cout << "sock endpoint: "<< sock.local_endpoint() << std::endl;

	std::mutex sock_mutex;

	std::vector<std::thread> threads;
	threads.reserve(threads_count);

	for (size_t i=0; i<threads_count; i++) threads.emplace_back([&io_service](){io_service.run();});

	udp::endpoint sender_endpoint;

	//std::vector<double> speed_tab; // restuls

	{
		std::lock_guard<std::mutex> lg(sock_mutex);
		for (size_t i=0; i<threads_count; i++)
		{
			sock.async_receive_from(
					boost::asio::buffer(data[i], max_length), sender_endpoint
					, [&sock, &sock_mutex, i](const boost::system::error_code &ec, size_t length){handler(ec, length, sock, data[i], sock_mutex);} );
		}
	}

	std::thread thread_stop(
		[] {

		//std::vector<double> speed_tab;

		for (long int sample=0; true; ++sample) {
			std::this_thread::sleep_for( std::chrono::milliseconds(1000) );

			g_timer.step();

			// [counter] read
			std::cout << "Loop. ";
			//std::cout << "Wire: RECV={" << g_timer << "}";
			std::cout << "Wire: RECV={" << g_timer.get_info() << "}";
			std::cout << "; ";
			std::cout << std::endl;
			}
	});

	thread_stop.join();
	for (auto &thread : threads) thread.join();
	return 0; // returns above
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

		/*
		std::vector<double> result_tab;
		for (int restult_run=0; restult_run<10; ++restult_run) {
			result_tab.push_back( run_tests(io_service, std::atoi(argv[1]), threads_count) );
		}
		sort(result_tab.begin(), result_tab.end());
		auto best = result_tab.at( result_tab.size()-1 );
		std::cout << "\n\n" << "##### typical: " << corrected_avg(result_tab) << " best: " << best << " Mb/s" << "\n" << std::endl;
		std::cout << corrected_avg(result_tab) << " " << best << std::endl;
		*/
		run_tests(io_service, std::atoi(argv[1]), threads_count);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
