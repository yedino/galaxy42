#include <iostream>
#include <thread>
#include <chrono>
#include <memory>
#include "c_net_node.hpp"

// argv[0] = remote ipv6
int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Usage: asio-test <ipv6>" << std::endl;
		return 1;
	}
	_info("strat main");
	std::unique_ptr<c_api_tr> transmission_api(new c_net_node);
	for (int i = 0; i < 5; ++i) {
		transmission_api->write_to_nym(std::string(argv[1]), std::string("1234567890"));
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	
	_info("****recv data****");
	size_t chunks_size = 0;
	std::vector<s_message> inbox(std::move(transmission_api->read_or_wait_for_data()));
	for (auto &a : inbox) {
		chunks_size += a.m_data.size();
	}
	_info(inbox.size() << " chunks");
	_info("chunks_size = " << chunks_size);
	for (auto &a : inbox) {
		std::cout << a.m_source_id << "   " << a.m_data << std::endl;
	}
	return 0;
}