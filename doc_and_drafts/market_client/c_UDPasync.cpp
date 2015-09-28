#include "c_UDPasync.hpp"
#include "c_log.hpp"

c_UDPasync::c_UDPasync(std::string host, std::string server_port, unsigned short local_port) :
	socket(io_service, udp::endpoint(udp::v6(), local_port)),
	service_thread(&c_UDPasync::run_service, this) {
	udp::resolver resolver(io_service);
	udp::resolver::query query(udp::v6(), host, server_port);
	server_endpoint = *resolver.resolve(query);
	Send("");
}

c_UDPasync::~c_UDPasync() {
	io_service.stop();
	service_thread.join();
}

void c_UDPasync::start_receive() {
	//socket.async_receive_from(boost::asio::buffer(recv_buffer), remote_endpoint,
	//	[this](std::error_code ec, std::size_t bytes_recvd){ this->handle_receive(ec, bytes_recvd); });

	socket.async_receive_from(boost::asio::buffer(recv_buffer),
							  remote_endpoint,
							  boost::bind(&c_UDPasync::handle_receive, this,
							  boost::asio::placeholders::error,
							  boost::asio::placeholders::bytes_transferred));
}

void c_UDPasync::handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred) {
	if (!error) {
		std::string message(recv_buffer.data(), recv_buffer.data() + bytes_transferred);
		incomingMessages.push(message);
		statistics.RegisterReceivedMessage(bytes_transferred);
	} else {
		Log::Error("c_UDPasync::handle_receive:", error);
	}

	start_receive();
}

void c_UDPasync::Send(std::string message) {
	socket.send_to(boost::asio::buffer(message), server_endpoint);
	statistics.RegisterSentMessage(message.size());
}

void c_UDPasync::run_service() {
	start_receive();
	while (!io_service.stopped()) {
		try {
			io_service.run();
		} catch (const std::exception& e) {
			Log::Warning("Client: network exception: ", e.what());
		} catch (...) {
			Log::Error("Unknown exception in client network thread");
		}
	}
}


