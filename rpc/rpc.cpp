#include "rpc.hpp"

void rpc::send_tcp_msg(const std::string &msg, const std::string addr, int port) {
	using namespace asio_node;

	// note: tcp_asio_node can both sending and recieving messeges
	std::unique_ptr<c_connection_base> sender_node(new c_tcp_asio_node(19000));

	c_network_message message;
	message.address_ip = addr;
	message.port = port;
	message.data = msg;

	sender_node->send(std::move(message));
}

void rpc::rpc_demo() {
	_info("Running rpc demo");

try {
	// create example localhost resiever on port 9042
	std::unique_ptr<c_connection_base> example_reciever(new asio_node::c_tcp_asio_node(9042));

	_info("sending example message to localhost:9042");
	// default addr = "::1", port = 9042
	send_tcp_msg("Hello there");

	c_network_message message2;
	// wait for message
	do {
		message2 = example_reciever->receive();
		std::this_thread::yield();
	} while (message2.data.empty());

	_dbg2("source ip " << message2.address_ip);
	_dbg2("source port " << message2.port);
	_dbg2("data " << message2.data);
} catch (std::exception &err) {
	std::cerr << err.what() << std::endl;
}
}
