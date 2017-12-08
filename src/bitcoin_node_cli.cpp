#include "bitcoin_node_cli.hpp"
#include <json.hpp>

bitcoin_node_cli::bitcoin_node_cli(const std::string &ip_address, unsigned short port)
:
	m_io_service(),
	m_btc_rpc_endpoint(boost::asio::ip::address_v4::from_string(ip_address), port),
	m_socket(m_io_service)
{
}

uint32_t bitcoin_node_cli::get_balance() {
	try {
		// TODO: bitcoin rpc pass
		std::string get_balance_request;
		get_balance_request += "POST / HTTP/1.1\r\n";
		get_balance_request += "Host: ::1\r\n";
		get_balance_request += "Connection: close\r\n";
		get_balance_request += "Authorization: Basic X19jb29raWVfXzowMmNmYzBlYjE2YWU1OTQzMDIxMWVlZTc0NTc0NGVhNTVkZTk4MjM2ZDkzNTU2YzE5MGMxNzEyNjgyNTQwNmNh\r\n";
		get_balance_request += "Content-Length: 48\r\n\r\n";
		get_balance_request += R"({"method":"getbalance","params":["*",0],"id":1})";
		get_balance_request += "\n";

		m_socket.connect(m_btc_rpc_endpoint);
		m_socket.send(boost::asio::buffer(get_balance_request));
		std::string receive_buffer(1024, '\0');
		size_t readed_bytes = m_socket.receive(boost::asio::buffer(&receive_buffer[0], receive_buffer.size()));

		receive_buffer.resize(readed_bytes);
		auto it = std::find(receive_buffer.begin(), receive_buffer.end(), '{'); // find begin of json data
		receive_buffer.erase(receive_buffer.begin(), it); // remove http POST data
		_mark(receive_buffer);

		// remove other json fields
		receive_buffer.erase(receive_buffer.begin()); // remove first '{'
		auto pos = receive_buffer.find(R"("result":)"); // find "result":
		if (pos == std::string::npos) return 0;
		receive_buffer.erase(pos, std::string(R"("result":)").size()); // remove "result":
		it = std::find_if(receive_buffer.begin(), receive_buffer.end(),
		                  [](unsigned char c) {
		                  		if(c == '.' || std::isdigit(c)) return false;
		                  		return true;
		                  }
		);
		receive_buffer.erase(it, receive_buffer.end());
		const std::string btc_amount_str = receive_buffer;

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
