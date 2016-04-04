#ifndef C_TCP_NETWORK_HPP
#define C_TCP_NETWORK_HPP

#include "../libs01.hpp"
#include "c_locked_queue.hpp"

using namespace boost::asio;

enum class packet_type : uint16_t { empty = 0,
                                    handshake = 200,
                                    public_key = 210,
                                    token_send = 220,
                                    contract = 230 };
class connection;

class c_TCPcommand {
  public:
    c_TCPcommand (const packet_type cmd_type, const std::string &data = "");

    packet_type get_type() const;
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

    c_TCPcommand(c_TCPcommand &) = delete;
};

class c_TCPasync {
    typedef std::map<ip::tcp::endpoint, std::shared_ptr<connection>> connection_map;
    typedef std::vector<std::reference_wrapper<c_TCPcommand>> cmd_wrapped_vector;
  public:
    c_TCPasync (unsigned short local_port = 30000);		// TODO This constructor need to set_target before use!
    c_TCPasync (const std::string &host, unsigned short server_port = 30000, unsigned short local_port = 30000);

    /// Setting new server target
    void set_target (const std::string &host_address, unsigned short server_port = 30000);

    /// Connection test
    /// wait - for up to seconds for host to be available
    std::shared_ptr<connection> connect (const std::string &host_address,
                                         unsigned short server_port,
                                         std::chrono::seconds wait = std::chrono::seconds(30));

    io_service& get_io_service() {
        return m_io_service;
    }
    bool is_connected ();

    /// add command that will be handled with this TCP connection
    void add_cmd(c_TCPcommand &cmd);
	/// send request by server_socket. Throw exception for not available protocols
    void send_cmd_request(packet_type type);
    void send_cmd_response(packet_type type, const std::string &packet);

    unsigned short get_server_port ();
    unsigned short get_local_port ();

    ~c_TCPasync();
  private:
    io_service m_io_service;
    unsigned short m_local_port;
    ip::address m_host_address;

    connection_map m_connections;

    ip::tcp::acceptor m_acceptor;	///< acceptor that listen for incoming packets

    std::atomic<bool> m_stop_flag;

    /// Asynchronously accept a new connection.
    void async_accept();

    /// Handler for new connections.
    void handle_accept(boost::system::error_code const& ec, std::shared_ptr<connection> conn);

    void server_read (ip::tcp::socket &socket);

    cmd_wrapped_vector m_available_cmd;
    cmd_wrapped_vector::iterator find_cmd(packet_type type);		// interesting, this is used only on declaration

	std::vector<std::thread> m_threads;
    void threads_maker (unsigned short num);
    std::mutex m_mtx;

    c_TCPasync(c_TCPasync &) = delete;
};

class connection {
  public:

    connection(io_service &io) : m_socket(io)
    { }

    ip::tcp::socket& get_socket() {
        return m_socket;
    }

    ip::tcp::endpoint get_remote_endpoint() const {
        return m_socket.remote_endpoint();
    }

  private:
    ip::tcp::socket m_socket;
};

#endif // C_TCP_NEWTORK_HPP
