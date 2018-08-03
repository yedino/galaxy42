#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <algorithm>
#include <cassert>
#include <mutex>
#include <array>
#include <type_traits>

#include "../../tools_helper.hpp"

using boost::asio::ip::udp;

using t_counter = uint64_t;

constexpr size_t max_length = 65535;
constexpr size_t max_threads = 8192;
size_t threads_count = 1;

std::array< char[max_length] , max_threads > data;

using buf_type = decltype(boost::asio::buffer(data[0], max_length));
std::vector<buf_type> buffers;
//std::vector<boost::asio::const_buffer> buffers;

c_timerfoo g_timer(20);

static void escape(const void* p) {
	asm volatile("" : : "g"(p) : "memory");
}

void handler(const boost::system::error_code &ec, size_t length, udp::socket &sock, buf_type buffer, std::mutex &sock_mutex)
{
	//escape(&(*(buffer.begin())));

	g_timer.add(1, length);

	if (ec)
	{
		static size_t counter = 0;
		std::cout << ec << std::endl;
		std::cout << ec.message() << std::endl;
		counter++;
	}

	std::lock_guard<std::mutex> lg(sock_mutex);

	sock.async_receive(
			buffer
			, [&sock, buffer, &sock_mutex](const boost::system::error_code &ec, size_t length){handler(ec, length, sock, buffer, sock_mutex);} );
}

double run_tests(boost::asio::io_service& io_service, unsigned short port, size_t threads_count)
{
	udp::socket sock(io_service, udp::endpoint(boost::asio::ip::address_v4::any(), port));

	std::cout << "sock endpoint: "<< sock.local_endpoint() << std::endl;

	std::mutex sock_mutex;

	std::vector<std::thread> threads;
	threads.reserve(threads_count);

	{
		std::lock_guard<std::mutex> lg(sock_mutex);
		for (size_t i=0; i<threads_count; i++)
		{
			sock.async_receive(
					buffers[i]
					, [&sock, &sock_mutex, i](const boost::system::error_code &ec, size_t length){handler(ec, length, sock, buffers[i], sock_mutex);} );
		}
	}

	for (size_t i=0; i<threads_count; i++) threads.emplace_back([&io_service](){std::cout<<std::this_thread::get_id()<<std::endl;io_service.run();});

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
		if (argc != 3)
		{
			std::cerr << "Usage: server <port> <number of threads>\n";
			return 1;
		}

		threads_count = std::stoi(argv[2]);
		if (threads_count > max_threads)
		{
			std::cerr << "Usage: server <port> <number of threads>\n";
			return 1;
		}

		buffers.reserve(threads_count);
		for (size_t i=0; i<threads_count; i++) buffers.emplace_back(data[i], max_length);

		boost::asio::io_service io_service;

		run_tests(io_service, std::stoi(argv[1]), threads_count);
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
