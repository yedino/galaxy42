#include <boost/asio.hpp>
#include <iostream>

#include "libs0.hpp"

using namespace boost;

void asiotest_udpserv() {
#if 0
	asio::io_service ios;

  if (1) {
		char inbuf_data[64];
		auto inbuf = asio::buffer( inbuf_data , std::extent<decltype(inbuf_data)>::value );
		asio::ip::udp::socket mysocket(ios); // active udp
		_note("bind");
		mysocket.open( asio::ip::udp::v4() );
		mysocket.bind( asio::ip::udp::endpoint( asio::ip::address_v4::any() , 9000 ) );
		asio::ip::udp::endpoint remote_ep;
		_note("receive");

		// TODO

	//	mysocket.async_receive

		/*size_t read_size = mysocket.receive_from( inbuf , remote_ep ); // ***
		_note("got data from remote " << remote_ep);
		_note("read: ["<<std::string(&inbuf_data[0],read_size)<<"]");
		*/
	}
#endif
}

void asiotest()
{
#if 0
	_mark("asiotest");

  int port_num = 3456;

//  asiotest_udpserv();

  asio::ip::tcp protocol_tcp = asio::ip::tcp::v4();
  asio::ip::udp protocol_udp = asio::ip::udp::v4();

	if (1) { // UDP server/client - anyway active socket
		char inbuf_data[64];
		auto inbuf = asio::buffer( inbuf_data , std::extent<decltype(inbuf_data)>::value );
		asio::ip::udp::socket mysocket(ios); // active udp
		_note("bind");
		mysocket.open( protocol_udp );
		mysocket.bind( asio::ip::udp::endpoint( asio::ip::address_v4::any() , 9000 ) );
		asio::ip::udp::endpoint remote_ep;
		_note("receive");
		size_t read_size = mysocket.receive_from( inbuf , remote_ep ); // ***
		_note("got data from remote " << remote_ep);
		_note("read: ["<<std::string(&inbuf_data[0],read_size)<<"]");
	}

	if (0) { // TCP server - part
		asio::ip::address ip_address = asio::ip::address_v4::any();
		asio::ip::tcp::endpoint ep(ip_address, port_num);

		asio::ip::tcp::socket mysocket(ios);

		asio::ip::tcp::acceptor myacceptor(ios);

		mysocket.open(protocol_tcp);
		myacceptor.open(protocol_tcp);
	}

	if (0) { // TCP client
		asio::ip::tcp::endpoint ep_dst_tcp( asio::ip::address::from_string("127.0.0.1") , 80 );
		asio::ip::tcp::socket tcpsend( ios );
		_note("open");
		tcpsend.open(protocol_tcp);
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

#endif
}

