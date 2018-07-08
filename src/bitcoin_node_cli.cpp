#include "bitcoin_node_cli.hpp"
#include <json.hpp>


c_curl_ptr::c_curl_ptr()
:	m_ptr(nullptr)
{
	m_ptr = curl_easy_init();
	if (m_ptr == nullptr) {
		throw std::runtime_error("CURL init error");
	}
}

c_curl_ptr::~c_curl_ptr() {
	curl_easy_cleanup(m_ptr);
}

CURL *c_curl_ptr::get_raw_ptr() const {
	return m_ptr;
}

//////////////////////////////////////////////////////

//this parametrs are static
std::unique_ptr<bitcoin_node_cli> bitcoin_node_cli::m_instance;
std::once_flag bitcoin_node_cli::m_once_flag;
bool bitcoin_node_cli::curl_initialized=false;


bitcoin_node_cli& bitcoin_node_cli::get_instance()
{
	std::call_once(m_once_flag, []{
		m_instance.reset(new bitcoin_node_cli);
	});

	return *m_instance.get();
}

bitcoin_node_cli::bitcoin_node_cli(const std::string &ip_address, unsigned short port)
:
	m_rpc_http_address("http://" + ip_address + ':' + std::to_string(port))
{
}

uint32_t bitcoin_node_cli::get_balance() const {
	std::lock_guard<std::mutex> lock(m_mutex);

	if(!curl_initialized){
		throw std::runtime_error("Error: curl is not initialized - in get balance");
	}

	const std::string request (R"({"method":"getbalance","params":["*",0],"id":1})");
	const std::string receive_data = send_request_and_get_response(request);
	_mark("Receive data " << receive_data);

	nlohmann::json json = nlohmann::json::parse(receive_data.c_str());
	double btc_amount = json.at("result").get<double>();
	_check(btc_amount<=21000000);
	_check(btc_amount>=0);
	return btc_amount * 100'000'000. ; // return balance in satoshi
}

std::string bitcoin_node_cli::get_new_address() const {
	std::lock_guard<std::mutex> lock(m_mutex);

	if(!curl_initialized){
		throw std::runtime_error("Error: curl is not initialized - in get new address");
	}

	const std::string request = R"({"method":"getnewaddress","params":[],"id":1})";
	std::string receive_data = send_request_and_get_response(request);
	_mark("Receive data " << receive_data);

	nlohmann::json json = nlohmann::json::parse(receive_data);
	return json.at("result").get<std::string>();
}

std::string bitcoin_node_cli::send_request_and_get_response(const std::string &request) const {
	c_curl_ptr curl;
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_URL, m_rpc_http_address.c_str());

	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_POSTFIELDSIZE, request.size());
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_POSTFIELDS, request.c_str());
	// TODO load user and pass from config
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_USERNAME, "bitcoinrpcUSERNAME");
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_PASSWORD, "bitcoinrpcPASSWORD");

	std::string receive_data;
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_WRITEDATA, &receive_data);
	curl_easy_setopt(curl.get_raw_ptr(), CURLOPT_WRITEFUNCTION, &bitcoin_node_cli::write_cb);

	CURLcode ret_code = curl_easy_perform(curl.get_raw_ptr());
	if(ret_code != CURLE_OK) {
		pfp_warn("curl_easy_perform() failed: " << curl_easy_strerror(ret_code));
		throw std::runtime_error("curl_easy_perform() failed: "s + curl_easy_strerror(ret_code));
	}
	return receive_data;
}

size_t bitcoin_node_cli::write_cb(void *ptr, size_t size, size_t nmemb, std::string *str) {
	const size_t data_size = size * nmemb; // size of received data
	if (data_size == 0) return data_size;
	str->append(static_cast<const char *>(ptr), data_size);
	return data_size;
}



