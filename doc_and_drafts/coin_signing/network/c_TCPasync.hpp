#ifndef C_TCP_NETWORK_HPP
#define C_TCP_NETWORK_HPP

#include "../libs01.hpp"
#include "c_locked_queue.hpp"

using namespace boost::asio;

enum class protocol : uint16_t { empty = 0,
								 handshake = 200,
                                 public_key = 210,
                                 token_send = 220,
                                 contract = 230 };

class c_TCPcommand {
  public:
    c_TCPcommand (const protocol cmd_type, const std::string &data = "");

	protocol get_type() const;
    void set_response(const std::string &data);

    void send_request (ip::tcp::socket &socket);
    void send_response (ip::tcp::socket &socket);
	void get_response (ip::tcp::socket &socket);

	bool has_message ();
	std::string pop_message ();

  private:
    protocol m_type;
    std::string m_response_data;
    c_locked_queue<std::string> incoming_box;
    std::mutex m_mtx;

    c_TCPcommand(c_TCPcommand &) = delete;
};

class c_TCPasync {
  public:
    c_TCPasync (unsigned short local_port = 30000);		// TODO This constructor need to set_target before use!
    c_TCPasync (const std::string &host, unsigned short server_port = 30000, unsigned short local_port = 30000);

    /// Setting new server target
    void set_target (const std::string &host, unsigned short server_port = 30000);
    /// Connection test
    /// wait - in seconds for host connection, and setting server target
    bool connect (std::chrono::seconds wait = std::chrono::seconds(30));
    bool is_connected ();

    /// add command that will be handled with this TCP connection
    void add_cmd(c_TCPcommand &cmd);
	/// send request by server_socket. Throw exception for not available protocols
	void send_cmd_request(protocol type);
    void send_cmd_response(protocol type, const std::string &packet);

    unsigned short get_server_port ();
    unsigned short get_local_port ();

    ~c_TCPasync();
  private:
    io_service m_io_service;
    unsigned short m_local_port;
    ip::address m_host_address;

	ip::tcp::socket m_local_socket;
    ip::tcp::acceptor m_acceptor;	///< acceptor that listen for incoming packets

    ip::tcp::socket m_server_socket;
    ip::tcp::endpoint m_server_endpoint;	///< host server endpoint for sending packets

    std::atomic<bool> m_stop_flag;
    void create_server ();
	void server_read (ip::tcp::socket socket);

    std::vector<std::reference_wrapper<c_TCPcommand>> m_available_cmd;
	decltype(m_available_cmd)::iterator find_cmd(protocol type);		// interesting, this is used only on declaration

	std::vector<std::thread> m_threads;
    void threads_maker (unsigned short num);
    std::mutex m_mtx;

    c_TCPasync(c_TCPasync &) = delete;
};


#endif // C_TCP_NEWTORK_HPP
