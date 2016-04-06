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

    /// Setting new server target
    void set_target (const std::string &host_address, unsigned short server_port = 30000);

    /// Connection test
    /// wait - for up to seconds for host to be available
    std::shared_ptr<c_connection> connect (const std::string &host_address,
                                           unsigned short server_port,
                                           std::chrono::seconds wait = std::chrono::seconds(30));

    io_service &get_io_service ();
    /// add command that will be handled with this TCP connection
    void add_cmd (c_TCPcommand &cmd);
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

    connection_map m_connections;
    ip::tcp::acceptor m_acceptor;	///< acceptor that listen for incoming packets

    /// Asynchronously accept a new connection.
    void async_accept ();
    /// Handler for new connections.
    void handle_accept (boost::system::error_code const& ec, std::shared_ptr<c_connection> conn);

    void server_read (ip::tcp::socket &socket);
    void handle_type_read (const boost::system::error_code &ec,
                           size_t bytes_read,
                           packet_type &type,
                           ip::tcp::socket &socket);
    void handle_resreq_read (const boost::system::error_code &ec,
                             size_t bytes_read,
                             char &re,
                             cmd_wrapped_vector::iterator cmd,
                             ip::tcp::socket &socket);

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

    c_connection(io_service &io) : m_socket(io)
    { }

    ip::tcp::socket& get_socket() {
        return m_socket;
    }

    ip::tcp::endpoint get_remote_endpoint() const {
        return m_socket.remote_endpoint();
    }
    ~c_connection() {
        DBG_MTX(dbg_mtx, "connection destructor");
        if (m_socket.is_open()) {
            boost::system::error_code ec;
            m_socket.shutdown(ip::tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
    }
  private:
    ip::tcp::socket m_socket;
    // ed_key remote_pubkey; // TODO
};

#endif // C_TCP_NEWTORK_HPP
