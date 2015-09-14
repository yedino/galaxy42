#include "c_connect.hpp"

c_connect::c_connect(std::string host, std::string port) :
	m_host(host), m_port(port)
{ }

c_connect::~c_connect() {
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

c_udp_link::c_udp_link(std::string host, std::string port,protocol_type ptype) :
	c_connect(host,port),
	s(io_service, udp::endpoint(udp::v4(), 0)) {
	m_ip = ptype;
    udp::resolver resolver(io_service);
	endpoint = *resolver.resolve({udp::v4(), m_host, m_port});
}

std::string c_udp_link::send_msg(std::string &msg) {

	std::cerr << "(udp) Sending:\n" << msg << std::endl;		//dbg
	s.send_to(boost::asio::buffer(msg.c_str(), msg.size()), endpoint);
	char reply[max_length-1];
	reply[max_length] = '\0';
	udp::endpoint sender_endpoint;
	size_t reply_length = s.receive_from(
		boost::asio::buffer(reply, max_length), sender_endpoint);
	std::cerr << "(udp) Reply:\n" << reply << std::endl;		//dbg
	return std::string(reply,reply_length);

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

c_tcp_link::c_tcp_link(std::string host, std::string port, protocol_type ptype) :
	c_connect(host,port),
	s(io_service) {
	m_ip = ptype;
    tcp::resolver resolver(io_service);
    boost::asio::connect(s, resolver.resolve({host, port}));
}

std::string c_tcp_link::send_msg(std::string &msg) {
	
	std::cerr << "(tcp) Sending:\n" << msg << std::endl;		//dbg
	boost::asio::write(s, boost::asio::buffer(msg.c_str(), msg.size()));

	char reply[max_length];
	size_t reply_length = boost::asio::read(s,
		boost::asio::buffer(reply, msg.size()/*request_length ???*/));
	std::cerr << "(tcp) Reply:\n" << std::string(reply,reply_length) << std::endl;		//dbg
    return std::string(reply,reply_length);
	
}
