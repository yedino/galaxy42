#include "bitcoin_node_cli.hpp"
#include <json.hpp>

bitcoin_node_cli::bitcoin_node_cli(const std::string &ip_address, unsigned short port)
:
	m_io_service(),
	m_btc_rpc_endpoint(boost::asio::ip::address_v4::from_string(ip_address), port)
{
}

uint32_t bitcoin_node_cli::get_balance() const {
	const std::string get_balance_request = [this] {
		std::string get_balance_request = generate_request_prototype();
		get_balance_request += R"({"method":"getbalance","params":["*",0],"id":1})";
		get_balance_request += "\n";
		return get_balance_request;
	}();

	std::string receive_data = send_request_and_get_response(get_balance_request);
	_mark("Receive data " << receive_data);

	auto it = std::find(receive_data.begin(), receive_data.end(), '{'); // find begin of json data
	receive_data.erase(receive_data.begin(), it); // remove http POST data
	std::setlocale(LC_ALL, "en_US.UTF-8");
	nlohmann::json json = nlohmann::json::parse(receive_data.c_str());

	double btc_amount = json.at("result").get<double>();
	return btc_amount * 100'000'000. ; // return balance in satoshi
}

std::string bitcoin_node_cli::generate_request_prototype() const {
	std::string request;
	request += "POST / HTTP/1.1\r\n";
	request += "Host: ::1\r\n";
	request += "Connection: close\r\n";
	request += "Authorization: Basic X19jb29raWVfXzowMmNmYzBlYjE2YWU1OTQzMDIxMWVlZTc0NTc0NGVhNTVkZTk4MjM2ZDkzNTU2YzE5MGMxNzEyNjgyNTQwNmNh\r\n";
	request += "Content-Length: 48\r\n\r\n";
	return request;
}

std::string bitcoin_node_cli::send_request_and_get_response(const std::string &request) const {
	boost::asio::ip::tcp::socket socket(m_io_service);
	socket.connect(m_btc_rpc_endpoint);
	socket.send(boost::asio::buffer(request));
	std::string receive_buffer(1024, '\0');
	size_t readed_bytes = socket.receive(boost::asio::buffer(&receive_buffer[0], receive_buffer.size()));
	receive_buffer.resize(readed_bytes);
	return receive_buffer;
}
