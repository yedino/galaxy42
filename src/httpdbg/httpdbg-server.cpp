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
#include <ctime>
#include <boost/algorithm/string.hpp>

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

server::server(boost::asio::io_service& io_service, short port,
		const c_tunserver & tunserver
  )
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      socket_(io_service)
      ,tunserver_( tunserver )
  {
    do_accept();
  }

void server::do_accept()
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

const std::string c_httpdbg_raport::header = "HTTP/1.x 200 OK\n"
"Content-Type: text/html; charset=UTF-8\n\n"
"<!DOCTYPE html>"
"<html>"
"<head>"
"<style>"
"table, th, td {"
"           border: 1px solid black;"
"}"
"</style>"
"</head>"
"<body>"
"<h1>Http debug server</h1>"
;

const std::string c_httpdbg_raport::footer = "</body></html>";

c_httpdbg_raport::c_httpdbg_raport(const c_tunserver & target)
	: m_target(target)
{ }

std::string c_httpdbg_raport::generate() {
	std::ostringstream out;
        time_t t = time(0);
        struct tm * now = localtime( & t);
        out << now->tm_mday << '.' << (now->tm_mon + 1) << '.' << (now->tm_year + 1900) << ' ';
        out << now->tm_hour << ':' << setfill('0') << setw(2) << now->tm_min << ':' << setfill('0') << setw(2) << now->tm_sec << "</br>";

	std::lock_guard<std::mutex> lg( m_target.get_my_mutex()  );

	out << HTML("Peer: size=") << HTML(m_target.m_peer.size()) << endl;
        out << "<table><tr><th>Peering adress</th><th>Hip</th><th>Pub</th><th>Limit points</th><th>Connection time</th><th>Packets send</th></tr>";
        for(auto it = m_target.m_peer.begin(); it != m_target.m_peer.end(); it++)
        {
                out << "<tr><th>";
                out << HTML(it->second->get_pip()) << "</th><th>";
                out << HTML(it->second->get_hip()) << "</th><th>";
                out << HTML(*(it->second->get_pub())) << "</th><th>";
                out << HTML(it->second->get_limit_points()) <<  "</th></tr>";
        }
        out << "</table>";
	out << "</br>Tunnel: size=" << HTML(m_target.m_tunnel.size()) << endl;
        out << "<table><tr><th>Hip</th><th>Debug</th><th>Serialize bin pubkey</th><th>Ipv6</th><th>Packet start ab</th><th>Packet start final</th><th>Tunnel state</th></tr>";
        for(auto it = m_target.m_tunnel.begin(); it != m_target.m_tunnel.end(); it++)
        {
                out << "<tr><th>";
                out << HTML(it->first) << "</th><th>";
                out << HTML(it->second->debug_this()) << "</th><th>";
                try
                {
                        out << HTML(it->second->get_IDe().get_serialize_bin_pubkey()) << "</th><th>";
                }catch(...)
                {
                        out << "</th><th>";
                }
                try
                {
                        out << HTML(it->second->get_IDe().get_ipv6_string_hexdot()) << "</th><th>";
                }catch(...)
                {
                        out << "</th><th>";
                }
                out << HTML(it->second->m_state) << "</th></tr>";
        }

	return header + out.str() + footer;
}

std::string c_httpdbg_raport::HTML(const std::string & s) {
        std::string ret(s);
        boost::replace_all(ret, "<", "&lt;");
        boost::replace_all(ret, ">", "&gt;");
	return ret;
}

c_httpdbg_server::c_httpdbg_server(int opt_port, const c_tunserver & tunserver):
        opt_port_(opt_port)
        ,tunserver_(tunserver)
{}

int c_httpdbg_server::run()
{
  try
  {
    server s(io_service_, opt_port_, tunserver_);
    io_service_.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}

void c_httpdbg_server::stop()
{
  try
  {
    io_service_.stop();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
}
