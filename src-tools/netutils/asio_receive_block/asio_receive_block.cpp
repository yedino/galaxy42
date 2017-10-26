#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <chrono>
#include <algorithm>
#include <cassert>

#include "../../tools_helper.hpp"

using boost::asio::ip::udp;

using t_counter = uint64_t;

enum { max_length = 65535 };


double server(boost::asio::io_service& io_service, unsigned short port)
{
	udp::socket sock(io_service, udp::endpoint(udp::v4(), port));

	t_counter received_bytes = 0;
	t_counter received_packets = 0;
	t_counter received_bytes_all = 0;
	t_counter received_packets_all = 0;
	auto start_time = std::chrono::steady_clock::now();
	auto start_time_totall = std::chrono::steady_clock::now();

	char data[max_length];
	udp::endpoint sender_endpoint;
	int count_interval=0; // to e.g. reset timer after few intervals
	int count_fulltest=0; // how many full tests

	std::vector<double> speed_tab; // restuls

	bool all_done=false;

	while (!all_done)
	{
		if (!received_packets) {
			start_time = std::chrono::steady_clock::now();
		}
		size_t length = sock.receive_from(
				boost::asio::buffer(data, max_length), sender_endpoint);
		received_packets++;
		received_bytes += length;  received_bytes_all += length;

		if (received_packets % 500000 == 0) {
			auto time_now = std::chrono::steady_clock::now();
			double sec = ( std::chrono::duration_cast<std::chrono::microseconds>(time_now - start_time) ).count() / (1000.*1000.);
			double sec_total = ( std::chrono::duration_cast<std::chrono::microseconds>(time_now - start_time_totall) ).count() / (1000.*1000.);
			double speed = ((received_bytes * 8) / (1000*1000)) / sec ;
			std::cout << "     " << "Receive packages:" << received_packets
				<< ", receive bytes:" << received_bytes
				<< ", speed:" << speed
				<< " Totall=" << (received_bytes_all*8 / sec_total / (1000.*1000.))
				<< " Mbits/s" << std::endl;
				//<< ", speed:" << ((static_cast<double>(received_bytes) * 8) / (1000*1000)) / sec << " Mbits/s" << std::endl;

			++count_interval;

			if ( count_interval == 2 ) {
				//std::cout << "(restarting counter)" << std::endl;
				++count_fulltest;
				speed_tab.push_back(speed);

				if ( count_fulltest== 10 ) {
					auto avg = corrected_avg(speed_tab);
					auto the_result = avg;
					std::cout << "\n" << "## Result = " << the_result << " ";
					std::cout << "(mediana: " << mediana(speed_tab)
						<< " nice_avg: " << corrected_avg(speed_tab) << ")";
					std::cout << "\n\n";
					// for(auto x : speed_tab) std::cout << x << " ";
					std::cout << std::endl;
					count_fulltest=0;
					speed_tab.clear();

					std::cout << avg <<std::endl;
					all_done=true;
					return the_result;
				}

				received_bytes = 0;
				received_packets = 0;
				start_time = std::chrono::steady_clock::now();

				count_interval=0;
			}
		}

	}

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

		std::vector<double> result_tab;
		for (int restult_run=0; restult_run<10; ++restult_run) {
			result_tab.push_back( server(io_service, std::atoi(argv[1])) );
		}
		sort(result_tab.begin(), result_tab.end());
		auto best = result_tab.at( result_tab.size()-1 );
		std::cout << "\n\n" << "##### typical: " << corrected_avg(result_tab) << " best: " << best << " Mb/s" << "\n" << std::endl;
		std::cout << corrected_avg(result_tab) << " " << best << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}

