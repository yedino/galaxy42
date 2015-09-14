#include "c_market_client.hpp"

bool check_cmd(const char* cmd, const char* request, size_t request_length);
bool check_cmd(const std::string cmd, const std::string request);

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

void c_market_client::start_market_session() {

	bool isover = false;
	while(!isover) {
		std::cout << "Enter message: ";
		char request[max_length];
		std::cin.getline(request, max_length);
		//cin.clear(); cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
		size_t request_length = std::strlen(request);
		
		if(std::string(request, request_length) == "quit") {
			isover = true;
			break;
		}	
		std::string command;
		if(check_cmd("register",request,request_length)) {
			if(cl_crypto->getCrypto_method() == ed25519) {
				command += "ed25519|";
			}
			command += cl_crypto->get_public_key() + ':' + std::string(request,request_length);
		}
		else {
			command = cl_crypto->sign(std::string(request,request_length)) + ':' + request;
		}
		//std::cout << command << std::endl;		//dbg
		std::string reply =  m_market_link->send_msg(command);
		std:: cout <<  "Reply is:\n" << reply << std::endl;
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
				char cmd_test[cmd_length+1] = {};		//+1 for null at the end
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
