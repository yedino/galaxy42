#include <boost/asio.hpp>
#include <iostream>

#include "libs0.hpp"

using namespace boost;

void asiotest()
{
	_mark("asiotest");

  int port_num = 3456;

	asio::io_service ios;

  asio::ip::address ip_address = asio::ip::address_v4::any();
  asio::ip::tcp::endpoint ep(ip_address, port_num);

	asio::ip::tcp protocol = asio::ip::tcp::v4();
	asio::ip::tcp::socket mysocket(ios);

	asio::ip::tcp::acceptor myacceptor(ios);

	mysocket.open(protocol);
	myacceptor.open(protocol);

	if (1) { // TCP client
		asio::ip::tcp::endpoint ep_dst_tcp( asio::ip::address::from_string("127.0.0.1") , 80 );
		asio::ip::tcp::socket tcpsend( ios );
		_note("open");
		tcpsend.open(protocol);
		_note("connect");
		tcpsend.connect( ep_dst_tcp ); // waits for ACK here
		asio::write( tcpsend , asio::buffer( std::string("GET /index.html HTTP/1.1\r\n\r\n") ) );

		char inbuf_data[64];
		auto inbuf = asio::buffer( inbuf_data , std::extent<decltype(inbuf_data)>::value );
		_note("read");
		auto read_size = size_t{ asio::read( tcpsend , inbuf ) };
		_note("read size " << read_size);
		_note("read: ["<<std::string(&inbuf_data[0],read_size)<<"]");
	}

}

