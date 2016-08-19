// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#if 0
#include "rpc.hpp"
#include "../libs0.hpp"

using namespace asio_node;

void send_tcp_msg(const std::string &msg, const std::string addr, int port) {
	using namespace asio_node;

	// note: tcp_asio_node can both sending and recieving messeges
	std::unique_ptr<c_connection_base> sender_node(new c_tcp_asio_node(19000));

	c_network_message message;
	message.address_ip = addr;
	message.port = port;
	message.data = msg;

	sender_node->send(std::move(message));
}

void rpc_demo() {
	_info("Running rpc demo");

try {
	c_rpc_server rpc_server(42000);
	rpc_server.register_function("example", rpc_example_function);

	_info("sending example message to localhost:42000");
	send_tcp_msg("example;arg1;arg2;arg3;arg4", "127.0.0.1", 42000);
	std::this_thread::sleep_for(std::chrono::seconds(1));

} catch (std::exception &err) {
	std::cerr << err.what() << std::endl;
}
}

void c_rpc_server::main_loop() {
	assert(m_stop_flag == false);
	while (!m_stop_flag) {
		auto message = m_connection_node->receive();
		if (!message.data.empty()) { // get RPC request
			auto it = std::find(message.data.begin(), message.data.end(), ';');
			if (it == message.data.end()) continue; // bad packet format (not found ';')
			*it = ' '; // remove first ';' character
			std::istringstream iss(message.data);
			std::string command, arguments;
			iss >> command;
			iss >> arguments;
			std::lock_guard<std::mutex> lg(m_command_map_mtx);
			if(m_command_map.find(command) != m_command_map.end()) { // found commend function in map
				m_command_map.at(command)(arguments); // call command function
			}
			else {
				_dbg1("not found function " << command);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

c_rpc_server::c_rpc_server(const unsigned int port)
:
	m_connection_node(std::make_unique<c_tcp_asio_node>(port)),
	m_stop_flag(false),
	m_work_thread(std::make_unique<std::thread>(&c_rpc_server::main_loop, this))
{
}

void c_rpc_server::register_function(const std::string &command_name, std::function<bool (const std::string &)> function) {
	std::lock_guard<std::mutex> lg(m_command_map_mtx);
	m_command_map.insert(std::pair<std::string, std::function<bool (std::string)>>(command_name, function));
}

c_rpc_server::~c_rpc_server() {
	m_stop_flag = true;
	m_work_thread->join();
}

bool rpc_example_function(const std::string &arguments) {
	std::cout << "Example rpc function" << std::endl;
	std::cout << "arguments: " << arguments << std::endl;
	return false;
}
#endif