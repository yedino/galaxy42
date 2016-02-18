#ifndef C_NETUSER_H
#define C_NETUSER_H
#define DBG_MTX(X,Y) do{X.lock();std::cout<< __func__<<":\t\t "<<Y<<std::endl;X.unlock();}while(0)

#include "c_user.hpp"
#include <atomic>
#include <boost/asio.hpp>
#include <thread>
#include <vector>

using namespace boost::asio;

class c_netuser : public c_user {
  public:
    c_netuser(std::string& username, int port = 30000);
    void send_token_bynet(const std::string &ip_address, int port = 30000);
    ~c_netuser();
  private:
    const int server_port;
    io_service m_io_service;
    ip::tcp::socket client_socket;
    ip::tcp::socket server_socket;
	ip::tcp::acceptor m_acceptor;
    void create_server();

	// protocol functions
    ed_key get_public_key_resp(ip::tcp::socket &socket_); ///< @param socket_ is connected socket, @return remote public key
	void send_public_key_req(ip::tcp::socket &socket_); ///< @param socket_ is connected socket
	void send_public_key_resp(ip::tcp::socket &socket_); ///< @param socket_ is connected socket
	void send_coin(ip::tcp::socket &socket_, const std::string &coin_data); ///< send one coin via connected @param socket_
	std::string recv_coin(ip::tcp::socket &socket_); ///< @return coin data from @param socket_

    void server_read(ip::tcp::socket socket_);
    void read_pubkey(ip::tcp::socket socket_);
    void read_token(ip::tcp::socket socket_);

    std::atomic<bool> m_stop_flag;

    enum { max_length = 1024 };
    char data_[max_length];

    std::vector<std::thread> m_threads;
    void threads_maker(unsigned);
    std::mutex dbg_mtx;
};

#endif // C_NETUSER_H
