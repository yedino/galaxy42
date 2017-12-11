#include "bitcoin_node_cli.hpp"
#include <json.hpp>

bitcoin_node_cli::bitcoin_node_cli(const std::string &ip_address, unsigned short port)
:
	m_io_service(),
	m_btc_rpc_endpoint(boost::asio::ip::address_v4::from_string(ip_address), port)
{
}

uint32_t bitcoin_node_cli::get_balance() const {
	try {
		// TODO: bitcoin rpc pass
		const std::string get_balance_request = [this] {
			std::string get_balance_request = generate_request_prototype();
			get_balance_request += R"({"method":"getbalance","params":["*",0],"id":1})";
			get_balance_request += "\n";
			return get_balance_request;
		}();

		std::string receive_data = send_request_and_get_response(get_balance_request);

		auto it = std::find(receive_data.begin(), receive_data.end(), '{'); // find begin of json data
		receive_data.erase(receive_data.begin(), it); // remove http POST data
		_mark(receive_data);

		// remove other json fields
		receive_data.erase(receive_data.begin()); // remove first '{'
		auto pos = receive_data.find(R"("result":)"); // find "result":
		if (pos == std::string::npos) return 0;
		receive_data.erase(pos, std::string(R"("result":)").size()); // remove "result":
		it = std::find_if(receive_data.begin(), receive_data.end(),
		                  [](unsigned char c) {
		                  		if(c == '.' || std::isdigit(c)) return false;
		                  		return true;
		                  }
		);
		receive_data.erase(it, receive_data.end());
		const std::string btc_amount_str = receive_data;

		_mark("btc_amount_str " << btc_amount_str);
		std::stringstream ss;
		ss << std::setprecision(20) << btc_amount_str;
		double btc_amount;
		ss >> btc_amount;

		return btc_amount * 10'000'000. ; // return balance in satoshi
	} catch (const std::exception &e) {
		_erro(e.what());
		abort();
		return 0;
	}
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
