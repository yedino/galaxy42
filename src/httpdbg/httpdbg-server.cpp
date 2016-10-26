//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <string>
#include <sstream>
#include <boost/asio.hpp>

#include "tunserver.hpp"

#include "httpdbg/httpdbg-server.hpp"

using boost::asio::ip::tcp;

using namespace std;


class session
  : public std::enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, const c_tunserver & tunserver)
    : socket_(std::move(socket))
    ,tunserver_(tunserver)
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            do_write(length);
          }
        });
  }

  void do_write(std::size_t length)
  {
		_UNUSED(length);
    auto self(shared_from_this());

		c_httpdbg_raport raport( tunserver_ );
    string reply=raport.generate();

    boost::asio::async_write(socket_,
    	boost::asio::buffer(reply, reply.size()),
    	// boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            // do_read();
//            socket_.
          }
        });
  }

  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];

  const c_tunserver & tunserver_;
};

class server
{
public:
  server(boost::asio::io_service& io_service, short port,
		const c_tunserver & tunserver
  )
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service)
      ,tunserver_( tunserver )
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
        [this](boost::system::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket_) , tunserver_ )->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_; ///< listen http on this socket
	const c_tunserver & tunserver_; ///< will debug this object [THREAD] must be locked by it's ->get_my_mutex()
};


c_httpdbg_raport::c_httpdbg_raport(const c_tunserver & target)
	: m_target(target)
{ }


std::string c_httpdbg_raport::generate() {
	std::lock_guard<std::mutex> lg( m_target.get_my_mutex()  );

	std::ostringstream out;

	out << HTML("Peer: size=") << HTML(m_target.m_peer.size());

	return out.str();
}

std::string c_httpdbg_raport::HTML(const std::string & s) {
	// TODO replace '<' -> "&lt;" , '>' -> "&gt;"
	return s;
}

int main_httpdbg(int opt_port, const c_tunserver & tunserver)
{
  try
  {
    boost::asio::io_service io_service;

    server s(io_service, opt_port, tunserver);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
