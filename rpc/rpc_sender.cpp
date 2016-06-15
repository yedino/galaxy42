#include "c_tcp_asio_node.hpp"
#include "rpc.hpp"


void send_rpc_request(const std::string &command_name, const std::string &arguments) {
	std::string request = command_name + ';' + arguments;
	_dbg1("request: " << request);
	send_tcp_msg(request, "127.0.0.1", 42000);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cout << "bad arguments" << std::endl;
		std::cout << "Usage: " << std::endl;
		std::cout << "./rpc_sender [command] [arguments]" << std::endl;
		return 1;
	}
	send_rpc_request(argv[1], argv[2]);
	return 0;
}
