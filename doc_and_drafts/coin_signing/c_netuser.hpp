#ifndef C_NETUSER_H
#define C_NETUSER_H

#include "c_user.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <vector>

using namespace boost::asio;

class c_netuser : public c_user {
  public:
    c_netuser(std::string& username);
    c_netuser(std::string&& username);
    void send_token_bynet(const std::string &ip_address);
    ~c_netuser();
  private:
    const int server_port = 55555;
    io_service m_io_service;
    ip::tcp::socket client_socket;
    ip::tcp::socket server_socket;
    vector<std::thread> m_threads;
    void threads_maker(unsigned);

};

#endif // C_NETUSER_H
