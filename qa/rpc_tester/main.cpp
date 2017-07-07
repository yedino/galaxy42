// ./rpc_tester --message \{\"cmd\"\:\"ping\"\,\"msg\"\:\"ping\"\}

#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <sodium.h>
#include <string>

namespace po = boost::program_options;

int main(int argc, char **argv) {
	try {
		if (sodium_init() == -1) {
			return 1;
		}

		uint16_t port = 0;
		std::string ip_srv_addr;
		std::string rpc_message;
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "Print help messages")
			("port", po::value<uint16_t>(&port)->default_value(42000), "RPC server port")
			("ip", po::value<std::string>(&ip_srv_addr)->default_value("127.0.0.1"), "RPC server address ipv4")
			("message", po::value<std::string>(&rpc_message)->required(), "RPC request");

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc << std::endl;
			return 0;
		}

		boost::asio::ip::address_v4 ip_srv = boost::asio::ip::address_v4::from_string(ip_srv_addr);
		std::cout << "RPC server address " << ip_srv << '\n';
		std::cout << "RPC server port " << port << '\n';

		std::array<unsigned char, crypto_auth_hmacsha512_KEYBYTES> hmac_key;
		hmac_key.fill(0x42);
		if (rpc_message.size() > std::numeric_limits<uint16_t>::max()) {
			std::cout << "RPC message is too long\n";
			return 1;
		}
		uint16_t message_size = rpc_message.size();
		std::array<unsigned char, 2> size_array;
		size_array.at(0) = message_size >> 8;
		size_array.at(1) = message_size & 0xFF;

		// calculate authenticator
		std::array<unsigned char, crypto_auth_hmacsha512_BYTES> authenticator;
		crypto_auth_hmacsha512(authenticator.data(),
							   reinterpret_cast<const unsigned char *>(rpc_message.data()),
							   rpc_message.size(),
							   hmac_key.data());

		// send request
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::socket socket(io_service);
		boost::asio::ip::tcp::endpoint endpoint(ip_srv, port);
		std::cout << "connect" << std::endl;
		socket.connect(endpoint);
		std::cout << "send size" << std::endl;
		boost::asio::write(socket, boost::asio::buffer(size_array.data(), size_array.size()));
		std::cout << "send message" << std::endl;
		boost::asio::write(socket, boost::asio::buffer(rpc_message.data(), rpc_message.size()));
		std::cout << "send authenticator" << std::endl;
		boost::asio::write(socket, boost::asio::buffer(authenticator.data(), authenticator.size()));

		// receive response
		std::cout << "wait for response" << std::endl;
		boost::asio::read(socket, boost::asio::buffer(&size_array.at(0), size_array.size()));
		message_size = size_array.at(0) << 8;
		message_size += size_array.at(1);
		std::cout << "read " << message_size << " bytes of message" << std::endl;
		std::string receive_message(message_size, 0);
		boost::asio::read(socket, boost::asio::buffer(&receive_message.at(0), receive_message.size()));
		std::cout << "RPC response: " << receive_message << '\n';
		std::cout << "receive authenticator" << std::endl;
		boost::asio::read(socket, boost::asio::buffer(&authenticator.at(0), authenticator.size()));

		int ret = crypto_auth_hmacsha512_verify(authenticator.data(),
												reinterpret_cast<const unsigned char *>(receive_message.data()),
												receive_message.size(),
												hmac_key.data());
		if (ret == -1) {
			std::cout << "verification error" << std::endl;
		} else {
			std::cout << "verification ok" << std::endl;
		}
	} catch (const std::exception &e) {
		std::cout << "catch exception: " << e.what() << '\n';
		return 1;
	} catch (...) {
		std::cout << "catch unknown exception type\n";
		return 1;
	}
}
