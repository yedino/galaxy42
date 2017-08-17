#include <boost/asio.hpp>
#include <iostream>

#include "libs0.hpp"

using namespace boost;

void asiotest()
{
	_mark("asiotest");

  int port_num = 3456;
  asio::ip::address ip_address = asio::ip::address_v4::any();
  asio::ip::tcp::endpoint ep(ip_address, port_num);


	asio::io_service ios;
	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket mysocket(ios);

	asio::ip::tcp::acceptor myacceptor(ios);

	mysocket.open(protocol);
	myacceptor.open(protocol);


	asio::ip::tcp::endpoint ep_dst_tcp( asio::ip::address::from_string("127.0.0.1") , 80 );
	asio::ip::tcp::socket tcpsend( ios );
	tcpsend.open(protocol);
	tcpsend.connect( ep_dst_tcp ); // waits for ACK here



}

