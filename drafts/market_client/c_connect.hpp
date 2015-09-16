#ifndef C_CONNECT_HPP
#define C_CONNECT_HPP
#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;

enum { max_length = 1024 };

typedef enum {
	tcp,
	udp
} protocol_type;

// may be useful
//static const char * EnumStrings[] = { "tcp", "udp"};
//const char * getTextForEnum(int enumVal){
//  return EnumStrings[enumVal];
//}

class c_connect {
public:
	c_connect(std::string host, std::string port);
	virtual ~c_connect();
	virtual std::string send_msg(std::string&){return "";} //TODO
protected:
	//c_connect (const c_connect &) = delete;
	c_connect (c_connect &&) = delete;
	void operator= (c_connect &&) = delete;
	void operator= (c_connect const &) = delete;

	protocol_type m_ip;
	std::string m_host;
	std::string m_port;
};

using boost::asio::ip::udp;
class c_udp_link : public c_connect {
public:
	c_udp_link(std::string host, std::string port,protocol_type ptype);
	std::string send_msg(std::string&);
private:
	boost::asio::io_service io_service;	
	udp::socket s;
	udp::endpoint endpoint;
};

using boost::asio::ip::tcp;
class c_tcp_link : public c_connect {
public:
	c_tcp_link(std::string host, std::string port, protocol_type ptype);
	std::string send_msg(std::string&);
private:
    boost::asio::io_service io_service;
	tcp::socket s;
};

#endif // C_CONNECT_HPP
