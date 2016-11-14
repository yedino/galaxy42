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
#include <fstream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include "tunserver.hpp"

#include "httpdbg/httpdbg-server.hpp"
#include <ctime>

using boost::asio::ip::tcp;

using namespace std;

class session
  : public enable_shared_from_this<session>
{
public:
  session(tcp::socket socket, const c_tunserver & tunserver)
    : socket_(move(socket))
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
        [this, self](boost::system::error_code ec, size_t length)
        {
          if (!ec)
          {
            do_write(length);
          }
        });
  }

  void do_write(size_t length)
  {
		_UNUSED(length);
    auto self(shared_from_this());

    c_httpdbg_raport raport( tunserver_ );
    vector<string> strs;
    boost::split(strs, data_, boost::is_any_of("/ "));

    string reply=raport.generate(strs[2]);

    boost::asio::async_write(socket_,
    	boost::asio::buffer(reply, reply.size()),
    	// boost::asio::buffer(data_, length),
        [this, self](boost::system::error_code ec, size_t /*length*/)
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
            make_shared<session>(move(socket_) , tunserver_ )->start();
          }

          do_accept();
        });
  }

c_httpdbg_raport::c_httpdbg_raport(const c_tunserver & target)
	: m_target(target)
{
    page_template = get_page_template("src/httpdbg/page_template.html");
}

string c_httpdbg_raport::get_page_template(const string &file_name){
    ostringstream out;
    string line;
    ifstream file (file_name);
    if (file.is_open())
    {
        while ( getline (file,line) )
            out << line << endl;
        file.close();
    }
    else cout << "Can not load page template file!";
    return out.str();
}

string c_httpdbg_raport::get_current_date(){
    ostringstream out;
    time_t t = time(0);
    struct tm * now = localtime( & t);
    out << now->tm_mday << '.' << (now->tm_mon + 1) << '.' << (now->tm_year + 1900) << ' ';
    out << now->tm_hour << ':' << setfill('0') << setw(2) << now->tm_min << ':' << setfill('0') << setw(2) << now->tm_sec;
    return out.str();
}

string c_httpdbg_raport::generate(string url) {
    ostringstream out;
    out << get_current_date() << "</br>" << endl;

    lock_guard<mutex> lg( m_target.get_my_mutex()  );

    if(url.size()!=32)
    {
        out << "<table><tr font-weight='bold'><th>Server name</th><th>Server hip</th></tr><tr><td>";
        out << m_target.m_my_name << "</td><td>" << m_target.m_my_hip.get_hip_as_string(true) << "</th></tr></table>";
        out << "</br><b>" << HTML("Number of peers: ") << HTML(m_target.m_peer.size()) << "</b></br>" << endl;
    }
    else
        out << "</br>" << "<a href=\"/\">Back</a></th><th>";
    if(m_target.m_peer.size())
    out << "<table><tr><th>Peering adress</th><th>Hip</th><th>Pub</th><th>Limit points</th><th>Data read</th><th>Packets read</th>"
           "<th>Data sent</th><th>Packets sent</th><th>Connection time</th><th>Data</th><th>Packets</th></tr>";
    string data = "";
    string chart = "";
    for(auto it = m_target.m_peer.begin(); it != m_target.m_peer.end(); it++)
    {
        string hip = it->first.get_hip_as_string(false);
        if (url.size() == 32 && hip.compare(url) != 0)
            continue;
        out << "<tr><td>";
        out << HTML(it->second->get_pip()) << "</td><td>";
        out << it->first.get_hip_as_string(true);
        if(url.size()!=32)
            out<< "</br><a href=\"" << hip << "\"> Details</a>";
        out << "</td><td>";
        out << HTML(it->second->get_pub()) << "</td><td>";
        out << HTML(it->second->get_limit_points()) <<  "</td><td>";
        out << HTML(it->second->get_stats().get_size_of_read_data()) << "</td><td>";
        out << HTML(it->second->get_stats().get_number_of_read_packets()) << "</td><td>";
        out << HTML(it->second->get_stats().get_size_of_sent_data()) << "</td><td>";
        out << HTML(it->second->get_stats().get_number_of_sent_packets()) << "</td><td>";
        out << HTML(it->second->get_stats().get_connection_time()) << "</td><td>";
        out << "<div id=\"cd_div" << hip << "\" style=\"width: 150px; height: 100px;\"></div></td><td>";
        out << "<div id=\"cp_div" << hip << "\" style=\"width: 150px; height: 100px;\"></div></td></tr>";
        data += it->second->get_stats().get_data_buffer().get_data_buffer_as_js_str(hip);
        data += it->second->get_stats().get_data_buffer().get_packets_buffer_as_js_str(hip);
        chart += it->second->get_stats().get_data_buffer().get_charts_as_js_str(hip, url.size()==32);
    }
    out << "</table></br>";
    if(url.size()!=32)
        out << "<b>Number of tunnels: " << HTML(m_target.m_tunnel.size()) << "</b>" << endl;
    if(m_target.m_tunnel.size())
        out << "<table><tr><th>Hip</th><th>Debug</th><th>Serialize bin pubkey</th><th>Ipv6</th>"
               "<th>Stream crypto ab nice name</th><th>Stream crypto ab boxer nounce</th><th>Stream crypto ab unboxer nounce</th>"
               "<th>Stream crypto final nice name</th><th>Stream crypto final boxer nounce</th><th>Stream crypto final unboxer nounce</th><th>Tunnel state</th></tr>";
    for(auto it = m_target.m_tunnel.begin(); it != m_target.m_tunnel.end(); it++)
    {
        string hip = it->first.get_hip_as_string(false);
        if (url.size() == 32 && hip.compare(url) != 0)
            continue;

        out << "<tr><td>";
        out << it->first.get_hip_as_string(true) << "</td><td>";
        out << HTML(it->second->debug_this()) << "</td><td>";
        try
        {
            out << HTML(it->second->get_IDe().get_serialize_bin_pubkey()) << "</td><td>";
        }catch(...)
        {
            out << "n/a</td><td>";
        }
        try
        {
            out << HTML(it->second->get_IDe().get_ipv6_string_hexdot()) << "</td><td>";
        }catch(...)
        {
            out << "n/a</td><td>";
        }
        if(it->second->m_stream_crypto_ab != NULL)
        {
            out << HTML(it->second->m_stream_crypto_ab->debug_this()) << "</td><td>";
            out << HTML(it->second->m_stream_crypto_ab->m_boxer->get_nonce()) << "</td><td>";
            out << HTML(it->second->m_stream_crypto_ab->m_unboxer->get_nonce()) << "</td><td>";
        }
        else
            out << "n/a</td><td>n/a</td><td>n/a</td><td>";
        if(it->second->m_stream_crypto_final != NULL)
        {
            out << HTML(it->second->m_stream_crypto_final->debug_this()) << "</td><td>";
            out << HTML(it->second->m_stream_crypto_final->m_boxer->get_nonce()) << "</td><td>";
            out << HTML(it->second->m_stream_crypto_final->m_unboxer->get_nonce()) << "</td><td>";
        }
        else
            out << "n/a</td><td>n/a</td><td>n/a</td><td>";
        out << HTML(it->second->m_state) << "</td></tr>";
    }
    out << "</table>";
    if(url.size()==32)
    {
        out << "<div id=\"bcd_div" << url << "\" style=\"width: 50%; height: 500px; float: left;\"></div>";
        out << "<div id=\"bcp_div" << url << "\" style=\"width: 50%; height: 500px; float: left;\"></div>";
    }
    boost::format page = boost::format(page_template) % data % chart % out.str();
    return page.str();
}

string c_httpdbg_raport::HTML(const string & s) {
        string ret(s);
        boost::replace_all(ret, "<", "&lt;");
        boost::replace_all(ret, ">", "&gt;");
	return ret;
}

c_httpdbg_server::c_httpdbg_server(int opt_port, const c_tunserver & tunserver):
        m_opt_port(opt_port)
        ,m_tunserver(tunserver)
{}

int c_httpdbg_server::run(){
    try
    {
        server s(m_io_service, m_opt_port, m_tunserver);
        m_io_service.run();
    }
    catch (exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}

void c_httpdbg_server::stop(){
    try
    {
        m_io_service.stop();
    }
    catch (exception& e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
}

void c_httpdbg_server::set_port(int port){
    m_opt_port = port;
}
