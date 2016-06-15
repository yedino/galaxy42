#ifndef C_TCP_NETWORK_HPP
#define C_TCP_NETWORK_HPP

#include "../libs01.hpp"
#include "c_locked_queue.hpp"
#include <boost/asio/spawn.hpp>
using namespace boost::asio;

enum class packet_type : uint16_t { empty = 0,
                                    handshake = 200,
                                    public_key = 210,
                                    token_send = 220,
                                    contract = 230 };
class c_connection;

class c_TCPcommand {
  public:
    c_TCPcommand (const packet_type cmd_type, const std::string &data = "");

    packet_type get_type () const;
    void set_response(const std::string &data);

    void send_request (ip::tcp::socket &socket);
    void send_response (ip::tcp::socket &socket);
	void get_response (ip::tcp::socket &socket);

	bool has_message ();
	std::string pop_message ();

  private:
    packet_type m_type;
    std::string m_response_data;
    c_locked_queue<std::string> incoming_box;
    std::mutex m_mtx;

    c_TCPcommand (c_TCPcommand &) = delete;
};

class c_TCPasync {
    typedef std::map<ip::tcp::endpoint, std::shared_ptr<c_connection>> connection_map;
    typedef std::vector<std::reference_wrapper<c_TCPcommand>> cmd_wrapped_vector;
  public:
    c_TCPasync (unsigned short local_port = 30000);		// TODO This constructor need to set_target before use!
    c_TCPasync (const std::string &host, unsigned short server_port = 30000, unsigned short local_port = 30000);

    unsigned int get_port();

    /// Setting new server target
    void set_target (const std::string &host_address, unsigned short server_port = 30000);

    /// Connection test
    /// wait - for up to seconds for host to be available
    /// If host is already connected returning existing connection
    std::shared_ptr<c_connection> connect (const std::string &host_address,
                                           unsigned short server_port,
                                           std::chrono::seconds wait = std::chrono::seconds(30));


    io_service &get_io_service ();
    /// add command that will be handled with this TCP connection
    void add_cmd (c_TCPcommand &cmd);
    /// finding and returning connection by given pubkey
    ///
    std::shared_ptr<c_connection> find_by_pubkey(const ed_key &pubkey);
    /// send request by server_socket. Throw exception for not available protocols
    void send_cmd_request (packet_type type, const std::string &host, unsigned short port);
    void send_cmd_response (packet_type type,
                            const std::string &host,
                            unsigned short port,
                            const std::string &packet);

    ~c_TCPasync();
  private:
    io_service m_io_service;
    unsigned short m_local_port;

    /// map [endpoint, shared_prt<c_conncetion>] for this tcp node
    connection_map m_connections;
    ip::tcp::acceptor m_acceptor;	///< acceptor that listen for incoming packets

    /// Asynchronously accept a new connection.
    void async_accept ();
    /// Handler for new connections.
    void handle_accept (boost::system::error_code const& ec, std::shared_ptr<c_connection> conn);

    /// type_read - 3 bytes struct that determine what to do with recieved packet
    struct __attribute__ ((__packed__)) {
        packet_type p_type = packet_type::empty;	// uint16_t
        uint8_t rs_action = 0;	// defines request or response action
    } type_read;
    void server_read (ip::tcp::socket &socket); 
    void handle_type_read (const boost::system::error_code &ec,
                           size_t bytes_read,
                           ip::tcp::socket &socket);

    /// Sending pubkey request for existing socket and waiting for response
    /// private member using in connect and handle accept
    ed_key get_remote_pubkey(ip::tcp::socket &socket);

    cmd_wrapped_vector m_available_cmd;
    cmd_wrapped_vector::iterator find_cmd(packet_type type);

	std::vector<std::thread> m_threads;
    void threads_maker (unsigned short num);
    std::atomic<bool> m_stop_flag;
    std::recursive_mutex m_mtx;

    c_TCPasync(c_TCPasync &) = delete;
};

class c_connection {
  public:
    c_connection(c_TCPasync &TCPasync);

    ip::tcp::socket& get_socket();
    ip::tcp::endpoint get_remote_endpoint() const;

    bool is_pubkey_known ();
    ed_key get_remote_pubkey();
    void set_remote_pubkey(const ed_key &remote_pubkey);

    ~c_connection();
  private:
    //c_TCPasync &m_TCPasync;	will be needed
    ip::tcp::socket m_socket;
    bool pubkey_known = false;
    ed_key m_remote_pubkey;
};

#endif // C_TCP_NEWTORK_HPP
