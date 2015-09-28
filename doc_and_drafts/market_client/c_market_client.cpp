#include "c_market_client.hpp"

bool check_cmd(const char* cmd, const char* request, size_t request_length);
bool check_cmd(const std::string cmd, const std::string request);
std::string get_request_usr(std::string req);
bool is_login_success(std::string reply);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
c_market_client::c_market_client(string host, string port,string ptype) {
	
	if(ptype == "udp") {
			m_market_link = std::shared_ptr<c_connect>(new c_udp_link(host,port,udp));
	}
	else if(ptype == "tcp") {
			m_market_link = std::shared_ptr<c_connect>(new c_tcp_link(host,port,tcp));
	}
	else {
			std::cerr << "market link fail : bad protocol" << std::endl;
	}
}

c_market_client::~c_market_client() {
	//delete m_market_link;
	//delete clr_crypto;
	//delete cle_crypto;
}

void c_market_client::encrypt_client(cryptosign_method crypt_m) {

	switch(crypt_m) {
		case RSA:
			cl_crypto = std::shared_ptr<c_RSA>(new c_RSA);
			break;
		case ed25519:
			cl_crypto =  std::shared_ptr<c_ed25519>(new c_ed25519);
			break;
		default:
			std::cerr << "client encryption fail : bad crypto method" << std::endl;
	}
}

bool verify_sign (const string &msg, const unsigned char *signature, const unsigned char *public_key) {
	return ed25519_verify(signature, reinterpret_cast<const unsigned char *>(msg.c_str()), msg.length(), public_key) != 0;
}

void c_market_client::start_market_session() {
	bool isover = false;
	std::string usr;
	std::cout << "Set your username for this session: ";	// TODO in future it will be nice to load your users from database.
	std::getline(std::cin,usr);
	c_client_user user(usr);


	while(!isover) {
		std::cout << "Enter message: ";

		std::string request;
		std::getline(std::cin,request);


		if(request == "quit") {
			isover = true;
			break;
		}
		std::string command;

		if(check_cmd("register",request)) {
			command += cl_crypto->get_public_key() + ':' + request + ':' + user.get_username();
		}
		else if(check_cmd("best_sell_offer",request) ||
				check_cmd("best_buy_offer",request) ||
				check_cmd("ping",request) ||
				check_cmd("moo",request) ||
				check_cmd("help",request)) {
			command += request;
		}
		else {
			std::string to_sign = request + ':' + user.get_username();
			command += cl_crypto->sign(to_sign) + ':' + request + ':' + user.get_username();
		}

		//std::cout << command << std::endl;		//dbg
		std::string reply =  m_market_link->send_msg(command);
		std:: cout <<  "Reply is:\n" << reply << std::endl;

	}
}
////////////////////////////////////////////////////////ASYNC VERSION//////////////////////////////////////////////////////////////////////////

c_UDPasync_client::c_UDPasync_client(std::string host, std::string server_port, unsigned short local_port) {
	m_market_link = std::shared_ptr<c_UDPasync>(new c_UDPasync(host, server_port, local_port));
}

c_UDPasync_client::~c_UDPasync_client() { }


//TODO this code shouldn't be doubled
void c_UDPasync_client::encrypt_client(cryptosign_method crypt_m) {

	switch(crypt_m) {
		case RSA:
			cl_crypto = std::shared_ptr<c_RSA>(new c_RSA);
			break;
		case ed25519:
			cl_crypto =  std::shared_ptr<c_ed25519>(new c_ed25519);
			break;
		default:
			std::cerr << "client encryption fail : bad crypto method" << std::endl;
	}
}

void c_UDPasync_client::start_async_session() {

	bool isover = false;
	std::string usr;
	std::string reply;
	std::cout << "Set your username for this session: ";	// TODO in future it will be nice to load your users from database.
	std::getline(std::cin,usr);
	c_client_user user(usr);

	while(!isover) {
		std::cout << "Enter message: ";

		std::string request;
		std::getline(std::cin,request);


		if(request == "quit") {
			isover = true;
			break;
		}
		std::string command;

		if(check_cmd("register",request)) {
			command += cl_crypto->get_public_key() + ':' + request + ':' + user.get_username();
		}
		else if(check_cmd("best_sell_offer",request) ||
				check_cmd("best_buy_offer",request) ||
				check_cmd("ping",request) ||
				check_cmd("moo",request) ||
				check_cmd("help",request)) {
			command += request;
		}
		else {
			std::string to_sign = request + ':' + user.get_username();
			command += cl_crypto->sign(to_sign) + ':' + request + ':' + user.get_username();
		}

		std::cout << "Send :\n" << command << std::endl;		//dbg
		m_market_link->Send(command);
		sleep(2);	//TODO change this to std
		if(m_market_link->HasMessages()) {
			reply = m_market_link->PopMessage();
			std::cout <<  "Reply :\n" << reply << std::endl;
		} else {
			std::cout << "No reply yet" << std::endl;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// C way
bool check_cmd(const char* cmd,const char* request, size_t request_length) {
		//const char* cmd = given_cmd.c_str();
		size_t cmd_length = std::strlen(cmd);
		for(size_t i = 0; i < request_length; ++i) {
			if(request[i] == ' ') {
				continue;
			}
			else if(request[i] == cmd[0]) {
				char cmd_test[cmd_length+1];		//+1 for null at the end
				strncpy ( cmd_test, request+i, (request_length-i >= cmd_length) ? cmd_length : 0 );
				//std::cout << "Ch - cmd : " << cmd << std::endl;		//dbg
				//std::cout << "Ch - cmd_test : " << cmd_test << std::endl;		//dbg
				if(strcmp(cmd,cmd_test) == 0) {
					return 1;
				}
			}
			else {
					return 0;
			}
		}
 		return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// simple C++ way   // but dont recognize spaces at the begining of request
bool check_cmd(const std::string cmd, const std::string request) {
	std::string subrequest = request.substr(0, request.find(' ', 0));
		return (subrequest == string(cmd));
}

bool is_login_success(std::string reply) {
	std::string correct = "veryfication OK: logged on";
	size_t found = reply.find(correct);
	if (found!=std::string::npos) {
		return true;
	}
	return false;
}

std::string get_request_usr(std::string req) {

	std::string user;
	std::string delimeter = " ";	// srv format: "sign:login user pass"
	std::size_t found = req.find(delimeter);
	if (found != std::string::npos) {
		user = req.substr(found);
		found = user.find(delimeter);
		if(found != std::string::npos) {
			user = user.substr(0,found);
			std::cout << "USER :: " << std::endl;
			return user;
		}
	}
	return "";
}
