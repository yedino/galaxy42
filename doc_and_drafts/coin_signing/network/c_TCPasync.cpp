#include "c_TCPasync.hpp"
#include "boost/bind.hpp"
//////////////////////////////////////////////////////////////////////////////////////////////////// TCPcommand

c_TCPcommand::c_TCPcommand(const packet_type cmd_type,
                           const std::string &data) : m_type(cmd_type),
                                                      m_response_data(data)
{ }

packet_type c_TCPcommand::get_type() const {
	return m_type;
}

void c_TCPcommand::set_response(const std::string &data) {
	m_response_data = data;
}

void c_TCPcommand::send_request(ip::tcp::socket &socket) {
    DBG_MTX(dbg_mtx, "send_request - protocol[" << static_cast<int>(m_type) << "]"); //dbg
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle

	std::vector<boost::asio::const_buffer> bufs;
	bufs.push_back(boost::asio::buffer(&m_type,type_bytes));
	bufs.push_back(boost::asio::buffer("q",1));

	std::size_t bytes_transfered = socket.write_some(bufs,ec);
	assert(bytes_transfered == (type_bytes+1));
}

void c_TCPcommand::send_response(ip::tcp::socket &socket) {
    DBG_MTX(dbg_mtx, "send_response - protocol[" << static_cast<int>(m_type) << "]"); // , data[" << m_response_data << "]"); //dbg
    assert(socket.is_open());

    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle
    unsigned short size_bytes = sizeof(uint64_t);	// size in bytes of packet data size
	uint64_t packet_size = m_response_data.size();

	std::vector<boost::asio::const_buffer> bufs;
	bufs.push_back(boost::asio::buffer(&m_type,type_bytes));
	bufs.push_back(boost::asio::buffer("s",1));
	bufs.push_back(boost::asio::buffer(&packet_size, size_bytes));
	bufs.push_back(boost::asio::buffer(m_response_data, packet_size));

	std::size_t bytes_transfered = socket.write_some(bufs,ec);
	assert(bytes_transfered == (type_bytes+size_bytes+packet_size+1));
}

void c_TCPcommand::get_response(ip::tcp::socket &socket) {
    //DBG_MTX(dbg_mtx, "get_response - start");	//dbg
    assert(socket.is_open());
    boost::system::error_code ec;
    uint64_t packet_size = 0;
    unsigned short size_bytes = sizeof(packet_size);	// size in bytes of packet data size

    size_t recieved_bytes = socket.read_some(buffer(&packet_size, size_bytes), ec);
    assert(recieved_bytes == size_bytes);

    const std::unique_ptr<char[]> packet_data(new char[packet_size]);
    recieved_bytes = socket.read_some(buffer(packet_data.get(), packet_size), ec);
    assert(recieved_bytes == packet_size);

    std::string recieved_str(packet_data.get(), packet_size);
    DBG_MTX(dbg_mtx, "get_response - recieved data[" << recieved_str << "]");	//dbg
    incoming_box.push(std::move(recieved_str));
}

bool c_TCPcommand::has_message() {
	return !incoming_box.empty();
}

std::string c_TCPcommand::pop_message() {
	std::lock_guard<std::recursive_mutex> lg(incoming_box.get_mutex());
	if (!has_message()) {
        throw std::logic_error("No message to pop");
	}
	return incoming_box.pop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////// TCPasync


c_TCPasync::c_TCPasync (unsigned short local_port) :
                                                     m_local_port(local_port),
                                                     m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v6(), m_local_port)),
                                                     m_stop_flag(false) {

    // Start accepting connections.
    async_accept();
    threads_maker(2);
}


c_TCPasync::c_TCPasync (const std::string &host,
						unsigned short server_port,
                        unsigned short local_port) :
                                                     m_local_port(local_port),
                                                     m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v6(), m_local_port)),
                                                     m_stop_flag(false) {

    connect(host,server_port);
    // Start accepting connections.
    async_accept();
    threads_maker(2);
}

unsigned int c_TCPasync::get_port() {
    return m_local_port;
}

std::shared_ptr<c_connection> c_TCPasync::connect(const std::string &host,
                                                  unsigned short host_port,
                                                  std::chrono::seconds wait) {
    DBG_MTX(dbg_mtx, "* connect *");
    boost::system::error_code ec;
    ip::address host_address = ip::address::from_string(host, ec);
    if (!host_address.is_v6()) {	// handling only ipv6 addr
        std::string msg = host_address.to_string();
        msg += ec.message() + " : is not valid IPv6 address";
        throw std::invalid_argument(msg);
    }

    ip::tcp::endpoint host_endpoint(host_address, host_port);
    auto ex_conn = m_connections.find(host_endpoint);
    if(ex_conn != m_connections.end()) {
        return ex_conn->second;			// returning existing connection
    }

    std::shared_ptr<c_connection> conn(new c_connection(*this));

    int attempts = 10;
    do {
        conn->get_socket().connect(host_endpoint, ec);
        if(ec) {
            DBG_MTX(dbg_mtx, "attempt " << attempts << " fail to connect");
            std::this_thread::sleep_for(wait/10);
            if(attempts == 0) {
                break;	// fail to connect in wait time
            }
        } else {
            DBG_MTX(dbg_mtx, "connect with " << host_address.to_string() << " on " << host_port << " port success");
            break;
        }
        attempts--;
    } while(true);

    conn->get_socket().set_option(ip::tcp::socket::reuse_address(true));
    conn->get_socket().set_option(ip::tcp::socket::linger(true, 0));
    ip::tcp::endpoint ep = conn->get_remote_endpoint();

    {
        std::lock_guard<std::recursive_mutex> lg(m_mtx);
        BOOST_ASSERT(m_connections.count(ep) == 0);
        m_connections[ep] = conn;
    }
    // Start reading
    server_read(conn->get_socket());

    // asking for remote pubkey
    conn->set_remote_pubkey(get_remote_pubkey(conn->get_socket()));
    return conn;
}

ed_key c_TCPasync::get_remote_pubkey(ip::tcp::socket &socket) {

    auto cmd_it = find_cmd(packet_type::public_key);
    if(cmd_it == m_available_cmd.end()) {
        std::string msg = "can't find ["
                          + std::to_string(static_cast<int>(packet_type::public_key))
                          + "] cmd available";

        throw std::invalid_argument(msg);
    }
    cmd_it->get().send_request(socket);
    std::string handle_pubkey;

    int wait = 5000, wait_left = 5000;	// time in milliseconds
    int step = 1;						//

    do {
        if(cmd_it->get().has_message()) {
            handle_pubkey = cmd_it->get().pop_message();
            std::cout << "Recieve response in: " << wait-wait_left << " milliseconds" << std::endl;
            break;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(step));
            }
        if(wait_left <= 0) {
            throw std::runtime_error("Fail to get response in wait time: " + std::to_string(wait/1000) + " seconds");
        }
        wait_left -= step;
    } while(true);

    return ed_key (reinterpret_cast<const unsigned char*>(handle_pubkey.c_str()),
                                                          handle_pubkey.size());
}

io_service &c_TCPasync::get_io_service() {
    return m_io_service;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_TCPasync::async_accept() {
    std::shared_ptr<c_connection> conn;
    conn.reset(new c_connection(*this));

    m_acceptor.async_accept(conn->get_socket(),
                            boost::bind(&c_TCPasync::handle_accept,
                                        boost::ref(*this),
                                        placeholders::error,
                                        conn));
}

void c_TCPasync::handle_accept(boost::system::error_code const& ec,
                            std::shared_ptr<c_connection> conn) {
    if (!ec) {

        // If there was no error, then we need to insert conn into the
        // the connection table, but first we want to set up the next
        // async_accept.
        std::shared_ptr<c_connection> old_conn(conn);

        conn.reset(new c_connection(*this));

        m_acceptor.async_accept(conn->get_socket(),
                                boost::bind(&c_TCPasync::handle_accept,
                                            boost::ref(*this),
                                            placeholders::error,
                                            conn));

        // Note that if we had multiple I/O threads, we would have to lock
        // before touching the map. yes

        ip::tcp::endpoint ep = old_conn->get_remote_endpoint();
        {
            std::lock_guard<std::recursive_mutex> lg(m_mtx);
            BOOST_ASSERT(m_connections.count(ep) == 0);
            m_connections[ep] = old_conn;
        }

        // Start reading.
        server_read(old_conn->get_socket());

        // asking for remote pubkey
        if(!old_conn->is_pubkey_known()) {
            std::cout << "[" << old_conn->get_remote_endpoint().port()
                      << "," << old_conn->get_remote_endpoint().address() << "]" << std::endl;
            //conn->set_remote_pubkey(get_remote_pubkey(conn->get_socket()));
        } else {
            std::cout << "KNOWN" << std::endl;
        }
        // asking for remote pubkey
        old_conn->set_remote_pubkey(get_remote_pubkey(old_conn->get_socket()));
    }
}


void c_TCPasync::server_read(ip::tcp::socket &socket) {
    assert(socket.is_open());

    type_read = {packet_type::empty, 0}; // set zeros
    unsigned short type_read_size = sizeof(type_read);

    async_read(socket,
               buffer(&type_read, type_read_size),
               boost::bind(&c_TCPasync::handle_type_read,
                           boost::ref(*this),
                           placeholders::error,
                           placeholders::bytes_transferred,
                           boost::ref(socket)));
}

void c_TCPasync::handle_type_read(const boost::system::error_code &ec,
                                  size_t bytes_read,
                                  ip::tcp::socket &socket) {

    DBG_MTX(dbg_mtx, "handle type read, bytes_read:" << bytes_read
                  << " ,type [" << static_cast<int>(type_read.p_type)
                  << "] , action [" << static_cast<char>(type_read.rs_action) << "]");
    if (ec) {
        DBG_MTX(dbg_mtx, "boost error: " << ec.message());
        return;
    } else if (bytes_read != sizeof(type_read)) {
        DBG_MTX(dbg_mtx, "bad bytes read");
        return;
    } else if (type_read.p_type == packet_type::empty) {
        DBG_MTX(dbg_mtx, "discard packet with empty protocol type");
        return;
    }

    auto cmd = find_cmd(type_read.p_type);
    if(cmd == m_available_cmd.end()) {
        DBG_MTX(dbg_mtx,"no available cmd found - type[" << static_cast<int>(type_read.p_type) << "]");
        return;
    };

    if (type_read.rs_action == 'q') {
        cmd->get().send_response(socket);
    } else if (type_read.rs_action == 's') {
        cmd->get().get_response(socket);
    } else {
        DBG_MTX(dbg_mtx,"discart packet with bad rs_action: [" << type_read.rs_action << "]"); //dbg
    }

    server_read(socket);	// back to async_read new packet
}

std::vector<std::reference_wrapper<c_TCPcommand>>::iterator c_TCPasync::find_cmd(packet_type type) {
    std::unique_lock<std::recursive_mutex> ul(m_mtx);
	auto cmd = std::find_if(m_available_cmd.begin(),
							m_available_cmd.end(),
							[type] (const std::reference_wrapper<c_TCPcommand> &i) {
                                if (i.get().get_type() == type) {
                                    return true;
                                }
                                return false;
                            });
	return cmd;
}

std::shared_ptr<c_connection> c_TCPasync::find_by_pubkey(const ed_key &pubkey) {
    std::unique_lock<std::recursive_mutex> ul(m_mtx);
    auto connection_pair = std::find_if(m_connections.begin(),
                                 m_connections.end(),
                                 [&pubkey] (const std::pair<ip::tcp::endpoint, std::shared_ptr<c_connection>> &i) {
                                    if (i.second->get_remote_pubkey() == pubkey) {
                                        return true;
                                    }
                                    return false;
                                 });
    if(connection_pair == m_connections.end()) {
        return nullptr;
    }
    return connection_pair->second;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// cmd's

void c_TCPasync::add_cmd(c_TCPcommand &cmd) {
    packet_type type = cmd.get_type();
    auto cmd_it = find_cmd(type);
    if(cmd_it == m_available_cmd.end()) {
        // DBG_MTX(dbg_mtx, "push_back [" << static_cast<int>(cmd.get_type()) << "] cmd"); //dbg
        m_available_cmd.push_back(std::ref(cmd));
    } else {
        DBG_MTX(dbg_mtx, "can't add cmd[" << static_cast<int>(type) << "] that already is available");
    }
}

void c_TCPasync::send_cmd_request(packet_type type, const std::string &host, unsigned short port) {
    auto cmd = find_cmd(type);
    if(cmd == m_available_cmd.end()) {
        throw std::invalid_argument("Protocol not allowed/added in this connection");
    }

    ip::tcp::endpoint host_endpoint(ip::address::from_string(host), port);
    auto ex_conn = m_connections.find(host_endpoint);
    if(ex_conn == m_connections.end()) {
        cmd->get().send_request(connect(host,port)->get_socket());
    } else {
        cmd->get().send_request(ex_conn->second->get_socket());
    }
}

void c_TCPasync::send_cmd_response(packet_type type,
                                   const std::string &host,
                                   unsigned short port,
                                   const std::string &packet) {
    auto cmd = find_cmd(type);
    cmd->get().set_response(packet);

    ip::tcp::endpoint host_endpoint(ip::address::from_string(host), port);
    auto ex_conn = m_connections.find(host_endpoint);
    if(ex_conn == m_connections.end()) {
        cmd->get().send_response(connect(host,port)->get_socket());
    } else {
        cmd->get().send_response(ex_conn->second->get_socket());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////// thread, destructor

c_TCPasync::~c_TCPasync() {
	m_stop_flag = true;
    m_acceptor.close();
    m_io_service.stop();
    for (auto &t : m_threads) {
        t.join();
	}
}

void c_TCPasync::threads_maker(unsigned short num) {
	m_threads.reserve(num);
    for (unsigned i = 0; i < num; ++i) {
        m_threads.emplace_back([this](){
            while (!m_stop_flag)
            {
                this->m_io_service.reset();
                this->m_io_service.run();
            }
        });
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////// C_CONNECTION

c_connection::c_connection(c_TCPasync &TCPasync) : //m_TCPasync(TCPasync),
    m_socket(TCPasync.get_io_service())
{ }

ip::tcp::socket &c_connection::get_socket() {
    return m_socket;
}

ip::tcp::endpoint c_connection::get_remote_endpoint() const {
    return m_socket.remote_endpoint();
}

bool c_connection::is_pubkey_known() {
    return pubkey_known;
}

ed_key c_connection::get_remote_pubkey() {
    if(pubkey_known) {
        return m_remote_pubkey;
    } else {
        throw std::runtime_error("Remote pubkey for this connection is not known");
    }
}

void c_connection::set_remote_pubkey(const ed_key &remote_pubkey) {
    DBG_MTX(dbg_mtx,"setting pubkey [" << remote_pubkey << "] for connection with: "
            << m_socket.remote_endpoint().address()
            << " on port " << m_socket.remote_endpoint().address());

    m_remote_pubkey = ed_key(remote_pubkey);
    pubkey_known = true;
}

c_connection::~c_connection() {
    if (m_socket.is_open()) {
        boost::system::error_code ec;
        m_socket.shutdown(ip::tcp::socket::shutdown_both, ec);
        m_socket.close(ec);
    }
}
