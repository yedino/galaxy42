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
	DBG_MTX(m_mtx, "send_request - protocol[" << (int)m_type << "]");
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle

	std::vector<boost::asio::const_buffer> bufs;
	bufs.push_back(boost::asio::buffer(&m_type,type_bytes));
	bufs.push_back(boost::asio::buffer("q",1));

	std::size_t bytes_transfered = socket.write_some(bufs,ec);
    DBG_MTX(m_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1); //dbg
	assert(bytes_transfered == (type_bytes+1));
}

void c_TCPcommand::send_response(ip::tcp::socket &socket) {
	DBG_MTX(m_mtx, "send_response - protocol[" << (int)m_type << "], data[" << m_response_data << "]");
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
    DBG_MTX(m_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1 << "+" << size_bytes << "+" << packet_size); //dbg
	assert(bytes_transfered == (type_bytes+size_bytes+packet_size+1));
}

void c_TCPcommand::get_response(ip::tcp::socket &socket) {
    DBG_MTX(m_mtx, "get_response - start");
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short size_bytes = sizeof(uint64_t);	// size in bytes of packet data size

    uint32_t packet_size = 0;
    size_t recieved_bytes = socket.read_some(buffer(&packet_size, size_bytes), ec);
    DBG_MTX(m_mtx,"recieved bytes: [" << recieved_bytes << "] size_bytes: [" << size_bytes << "]");
    assert(recieved_bytes == 8);

    const std::unique_ptr<char[]> packet_data(new char[packet_size]);
    recieved_bytes = socket.read_some(buffer(packet_data.get(), packet_size), ec);
    DBG_MTX(m_mtx,"recieved bytes: [" <<recieved_bytes << "] recieved_size: [" << packet_size << "]");
    assert(recieved_bytes == packet_size);

    std::string recieved_str(packet_data.get(), packet_size);
    DBG_MTX(m_mtx, "get_response - recieved[" << recieved_str << "]");
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

    boost::system::error_code ec;
    m_host_address = ip::address::from_string(host, ec);

    connect(host,server_port);
    // Start accepting connections.
    async_accept();
    threads_maker(2);
}

std::shared_ptr<connection> c_TCPasync::connect(const std::string &host,
                                                unsigned short host_port,
                                                std::chrono::seconds wait) {
    DBG_MTX(m_mtx, "* connect *");
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

    std::shared_ptr<connection> conn(new connection(get_io_service()));

    int attempts = 10;
    do {
        conn->get_socket().connect( host_endpoint, ec);
        if(ec) {
            DBG_MTX(m_mtx, "attempt " << attempts << " fail to connect");
            std::this_thread::sleep_for(wait/10);
            if(attempts == 0) {
                break;	// fail to connect in wait time
            }
        } else {
            DBG_MTX(m_mtx, "connect with " << m_host_address.to_string() << " on " << host_port << " port success");
            break;
        }
        attempts--;
    } while(true);

    conn->get_socket().set_option(ip::tcp::socket::reuse_address(true));
    conn->get_socket().set_option(ip::tcp::socket::linger(true, 0));

    ip::tcp::endpoint ep = conn->get_remote_endpoint();
    BOOST_ASSERT(m_connections.count(ep) == 0);
    m_mtx.lock();
    m_connections[ep] = conn;
    m_mtx.unlock();

    // Start reading.
    //server_read(conn->get_socket());

    return conn;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_TCPasync::async_accept() {
    std::shared_ptr<connection> conn;
    conn.reset(new connection(get_io_service()));

    m_acceptor.async_accept(conn->get_socket(),
        boost::bind(&c_TCPasync::handle_accept,
                    boost::ref(*this),
                    placeholders::error,
                    conn));
}

void c_TCPasync::handle_accept(boost::system::error_code const& ec,
                            std::shared_ptr<connection> conn) {
    if (!ec) {
        // If there was no error, then we need to insert conn into the
        // the connection table, but first we want to set up the next
        // async_accept.
        std::shared_ptr<connection> old_conn(conn);

        conn.reset(new connection(get_io_service()));

        m_acceptor.async_accept(conn->get_socket(),
            boost::bind(&c_TCPasync::handle_accept,
                        boost::ref(*this),
                        placeholders::error,
                        conn));

        // Note that if we had multiple I/O threads, we would have to lock
        // before touching the map.
        ip::tcp::endpoint ep = old_conn->get_remote_endpoint();
        BOOST_ASSERT(m_connections.count(ep) == 0);

        m_mtx.lock();
        m_connections[ep] = old_conn;
        m_mtx.unlock();

        // Start reading.
        server_read(old_conn->get_socket());
    }
}

void c_TCPasync::server_read(ip::tcp::socket &socket) {
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short prototype_size = sizeof(uint16_t);

    //ip::tcp::socket socket(std::move(socket_));

    while (!ec && !m_stop_flag) {
        assert(socket.is_open());
        packet_type type = packet_type::empty;
        DBG_MTX(m_mtx, "wait for data to read some");
        socket.read_some(buffer(&type, prototype_size), ec);
        if(type == packet_type::empty) {
			DBG_MTX(m_mtx, "discard packet with empty protocol type");
			break;
		}
        auto cmd = find_cmd(type);
        if(cmd == m_available_cmd.end()) {
			DBG_MTX(m_mtx,"server read: no available cmd found - type[" << static_cast<int>(type) << "]");
            break;
        };
        char re = '\0';
		socket.read_some(buffer(&re, 1), ec);	// request or response msg
         DBG_MTX(m_mtx,"req or res: " << re); //dbg
        if (re == 'q') {
            DBG_MTX(m_mtx,"server read: get request"); //dbg
            cmd->get().send_response(socket);
        } else if (re == 's') {
            DBG_MTX(m_mtx,"server read - get response"); //dbg
            cmd->get().get_response(socket);
        }
    }
    socket.close();
}

std::vector<std::reference_wrapper<c_TCPcommand>>::iterator c_TCPasync::find_cmd(packet_type type) {
	m_mtx.lock();
	auto cmd = std::find_if(m_available_cmd.begin(),
							m_available_cmd.end(),
							[type] (const std::reference_wrapper<c_TCPcommand> &i) {
		if(i.get().get_type() == type) {
			return true;
		}
		return false;
	});
	m_mtx.unlock();
	return cmd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// cmd's

void c_TCPasync::add_cmd(c_TCPcommand &cmd) {
    packet_type type = cmd.get_type();
    auto cmd_it = find_cmd(type);
    if(cmd_it == m_available_cmd.end()) {
        DBG_MTX(m_mtx, "push_back [" << static_cast<int>(cmd.get_type()) << "] cmd");
        m_available_cmd.push_back(std::ref(cmd));
    } else {
        DBG_MTX(m_mtx, "can't add cmd[" << static_cast<int>(type) << "] that already is available");
    }
}

void c_TCPasync::send_cmd_request(packet_type type) {
    auto cmd = find_cmd(type);
    if(cmd == m_available_cmd.end()) {
        throw std::invalid_argument("Protocol not allowed/added in this connection");
    };
    cmd->get().send_request(m_connections.begin()->second->get_socket());
}

void c_TCPasync::send_cmd_response(packet_type type, const std::string &packet) {
    auto cmd = find_cmd(type);
    cmd->get().set_response(packet);
    cmd->get().send_response(m_connections.begin()->second->get_socket());
}

//////////////////////////////////////////////////////////////////////////////////////////////////// thread, destructor

c_TCPasync::~c_TCPasync() {
	m_stop_flag = true;
	m_io_service.stop();
	for (auto &t : m_threads) {
		t.join();
	}
}

void c_TCPasync::threads_maker(unsigned short num) {
	m_threads.reserve(num);
    for (unsigned i = 0; i < num; ++i) {
        // DBG_MTX(dbg_mtx,"make " << i << " thread"); // dbg
        m_threads.emplace_back([this](){
            while (!m_stop_flag)
            {
                this->m_io_service.reset();
                this->m_io_service.run();
            }
            // DBG_MTX(dbg_mtx, "end of thread"); // dbg
        });
    }
}
