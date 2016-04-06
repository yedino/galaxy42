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
    DBG_MTX(dbg_mtx, "send_request - protocol[" << (int)m_type << "]");
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle

	std::vector<boost::asio::const_buffer> bufs;
	bufs.push_back(boost::asio::buffer(&m_type,type_bytes));
	bufs.push_back(boost::asio::buffer("q",1));

	std::size_t bytes_transfered = socket.write_some(bufs,ec);
    DBG_MTX(dbg_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1); //dbg
	assert(bytes_transfered == (type_bytes+1));
}

void c_TCPcommand::send_response(ip::tcp::socket &socket) {
    DBG_MTX(dbg_mtx, "send_response - protocol[" << (int)m_type << "], data[" << m_response_data << "]");
    assert(socket.is_open());

    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle

    if(m_response_data == "") {		// handling non set response data
        packet_type type = packet_type::empty;
        std::size_t bytes_transfered = socket.write_some(boost::asio::buffer(&type, type_bytes),ec);
        assert(bytes_transfered == type_bytes);
    }

    unsigned short size_bytes = sizeof(uint64_t);	// size in bytes of packet data size
	uint64_t packet_size = m_response_data.size();

	std::vector<boost::asio::const_buffer> bufs;
	bufs.push_back(boost::asio::buffer(&m_type,type_bytes));
	bufs.push_back(boost::asio::buffer("s",1));
	bufs.push_back(boost::asio::buffer(&packet_size, size_bytes));
	bufs.push_back(boost::asio::buffer(m_response_data, packet_size));

	std::size_t bytes_transfered = socket.write_some(bufs,ec);
    DBG_MTX(dbg_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1 << "+" << size_bytes << "+" << packet_size); //dbg
	assert(bytes_transfered == (type_bytes+size_bytes+packet_size+1));
}

void c_TCPcommand::get_response(ip::tcp::socket &socket) {
    DBG_MTX(dbg_mtx, "get_response - start");
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short size_bytes = sizeof(uint64_t);	// size in bytes of packet data size

    uint32_t packet_size = 0;
    DBG_MTX(dbg_mtx,"size_bytes: [" << size_bytes << "]");
    size_t recieved_bytes = socket.read_some(buffer(&packet_size, size_bytes), ec);
    DBG_MTX(dbg_mtx,"recieved bytes: [" << recieved_bytes << "] size_bytes: [" << size_bytes << "]");
    assert(recieved_bytes == 8);

    const std::unique_ptr<char[]> packet_data(new char[packet_size]);
    recieved_bytes = socket.read_some(buffer(packet_data.get(), packet_size), ec);
    DBG_MTX(dbg_mtx,"recieved bytes: [" <<recieved_bytes << "] recieved_size: [" << packet_size << "]");
    assert(recieved_bytes == packet_size);

    std::string recieved_str(packet_data.get(), packet_size);
    DBG_MTX(dbg_mtx, "get_response - recieved[" << recieved_str << "]");
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
    threads_maker(1);
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
    threads_maker(1);
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

    std::shared_ptr<c_connection> conn(new c_connection(get_io_service()));

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
    return conn;
}

io_service &c_TCPasync::get_io_service() {
    return m_io_service;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_TCPasync::async_accept() {
    std::shared_ptr<c_connection> conn;
    conn.reset(new c_connection(get_io_service()));

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

        conn.reset(new c_connection(get_io_service()));

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
    }
}


void c_TCPasync::server_read(ip::tcp::socket &socket) {
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short prototype_size = sizeof(uint16_t);
    static packet_type type = packet_type::empty;

    async_read(socket,
               buffer(&type, prototype_size),
               boost::bind(&c_TCPasync::handle_type_read,
                           boost::ref(*this),
                           placeholders::error,
                           placeholders::bytes_transferred,
                           boost::ref(type),
                           boost::ref(socket)));
}

void c_TCPasync::handle_type_read(const boost::system::error_code &ec,
                                  size_t bytes_read,
                                  packet_type &type,
                                  ip::tcp::socket &socket) {
    DBG_MTX(dbg_mtx, "handle type read, bytes_read:" << bytes_read << " ,type [" << static_cast<int>(type) << "]");
    if (ec) {
        DBG_MTX(dbg_mtx, "boost error: " << ec.message());
        return;
    }
    if (bytes_read != sizeof(uint16_t)) {
        DBG_MTX(dbg_mtx, "bad bytes read");
        return;
    }
    if (type == packet_type::empty) {
        DBG_MTX(dbg_mtx, "discard packet with empty protocol type");
        return;
    }
    auto cmd = find_cmd(type);
    if(cmd == m_available_cmd.end()) {
        DBG_MTX(dbg_mtx,"no available cmd found - type[" << static_cast<int>(type) << "]");
        return;
    };

    static char re = '\0';
    async_read(socket,
               buffer(&re, 1),
               boost::bind(&c_TCPasync::handle_resreq_read,
                           boost::ref(*this),
                           placeholders::error,
                           placeholders::bytes_transferred,
                           boost::ref(re),
                           cmd,
                           boost::ref(socket)));
}

void c_TCPasync::handle_resreq_read(const boost::system::error_code &ec,
                                    size_t bytes_read,
                                    char &re,
                                    cmd_wrapped_vector::iterator cmd,
                                    ip::tcp::socket &socket) {

    DBG_MTX(dbg_mtx, "handle resreq read, bytes_read:" << bytes_read << ", re: " << re);

    if (ec) {
        DBG_MTX(dbg_mtx, "boost error: " << ec.message());
        return;
    }
    if (bytes_read != sizeof(char)) {
        DBG_MTX(dbg_mtx, "bad bytes read");
        return;
    }
    DBG_MTX(dbg_mtx,"req or res: " << re); //dbg
    if (re == 'q') {
        DBG_MTX(dbg_mtx,"server read: get request"); //dbg
        cmd->get().send_response(socket);
    } else if (re == 's') {
        DBG_MTX(dbg_mtx,"server read - get response"); //dbg
        cmd->get().get_response(socket);
    } else {
        DBG_MTX(dbg_mtx,"server read - bad re?"); //dbg
    }
    server_read(socket);
}

std::vector<std::reference_wrapper<c_TCPcommand>>::iterator c_TCPasync::find_cmd(packet_type type) {
    std::unique_lock<std::recursive_mutex> ul(m_mtx);
	auto cmd = std::find_if(m_available_cmd.begin(),
							m_available_cmd.end(),
							[type] (const std::reference_wrapper<c_TCPcommand> &i) {
                                if(i.get().get_type() == type) {
                                    return true;
                                }
                                return false;
                            });
	return cmd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// cmd's

void c_TCPasync::add_cmd(c_TCPcommand &cmd) {
    packet_type type = cmd.get_type();
    auto cmd_it = find_cmd(type);
    if(cmd_it == m_available_cmd.end()) {
        DBG_MTX(dbg_mtx, "push_back [" << static_cast<int>(cmd.get_type()) << "] cmd");
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
    DBG_MTX(dbg_mtx,"TCPasync dbg_mutex stopped: " << m_io_service.stopped());
    for (auto &t : m_threads) {
        DBG_MTX(dbg_mtx,"t joinable: " << t.joinable());
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
