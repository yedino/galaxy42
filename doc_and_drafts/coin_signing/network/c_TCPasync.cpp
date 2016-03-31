#include "c_TCPasync.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////// TCPcommand

c_TCPcommand::c_TCPcommand(const protocol cmd_type, const std::string &data) : m_type(cmd_type),
                                                                               m_response_data(data)
{ }

protocol c_TCPcommand::get_type() const {
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
	// DBG_MTX(m_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1); //dbg
	assert(bytes_transfered == (type_bytes+1));
}

void c_TCPcommand::send_response(ip::tcp::socket &socket) {
	DBG_MTX(m_mtx, "send_response - protocol[" << (int)m_type << "], data[" << m_response_data << "]");
    assert(socket.is_open());

    boost::system::error_code ec;
    unsigned short type_bytes = sizeof(uint16_t);	// size in bytes of protocol tyle

    if(m_response_data == "") {		// handling non set response data
        protocol type = protocol::empty;
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
	// DBG_MTX(m_mtx,"transfered: " << bytes_transfered << " =?= " << type_bytes << "+" << 1 << "+" << size_bytes << "+" << packet_size); //dbg
	assert(bytes_transfered == (type_bytes+size_bytes+packet_size+1));
}

void c_TCPcommand::get_response(ip::tcp::socket &socket) {
    DBG_MTX(m_mtx, "get_response - start");
    assert(socket.is_open());
    boost::system::error_code ec;
    unsigned short size_bytes = sizeof(uint64_t);	// size in bytes of packet data size

    uint32_t packet_size = 0;
    size_t recieved_bytes = socket.read_some(buffer(&packet_size, size_bytes), ec);
    assert(recieved_bytes == size_bytes);

    const std::unique_ptr<char[]> packet_data(new char[packet_size]);
    recieved_bytes = socket.read_some(buffer(packet_data.get(), packet_size), ec);
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
		throw std::logic_error("No messages to pop");
	}
	return incoming_box.pop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////// TCPasync


c_TCPasync::c_TCPasync (unsigned short local_port) :
                                                     m_local_port(local_port),
                                                     m_local_socket(m_io_service),
                                                     m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v6(), m_local_port)),
                                                     m_server_socket(m_io_service),
                                                     m_stop_flag(false) {

    create_server();
    threads_maker(2);
}


c_TCPasync::c_TCPasync (const std::string &host,
						unsigned short server_port,
                        unsigned short local_port) :
                                                     m_local_port(local_port),
                                                     m_local_socket(m_io_service),
                                                     m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v6(), m_local_port)),
                                                     m_server_socket(m_io_service),
                                                     m_stop_flag(false) {

    boost::system::error_code ec;
    m_host_address = ip::address::from_string(host, ec);

    set_target(host,server_port);
    create_server();
    threads_maker(2);
}

void c_TCPasync::set_target(const std::string &host, unsigned short server_port) {
    m_server_socket.close();

    boost::system::error_code ec;
    m_host_address = ip::address::from_string(host, ec);
    m_server_endpoint = ip::tcp::endpoint(m_host_address, server_port);
    if (!connect()) {
        DBG_MTX(m_mtx, "connection with host: success");
    } else {
        DBG_MTX(m_mtx, "connection with host: fail");
        throw std::invalid_argument("unable connect to host");
    }
}

bool c_TCPasync::connect(std::chrono::seconds wait) {
    DBG_MTX(m_mtx, "* check connection *");

    boost::system::error_code ec;

    if (!m_host_address.is_v6()) {	// for now handling only ipv6 addr
        std::string msg = m_host_address.to_string();
        msg += ec.message() + " : is not valid IPv6 address";
        throw std::invalid_argument(msg);
    }

    int attempts = 5;
    do {
        m_server_socket.connect(m_server_endpoint, ec);
        std::this_thread::sleep_for(wait/5);
        if(ec) {
            DBG_MTX(m_mtx, "attempt " << attempts << " fail to connect");
            if(attempts == 0) {
                return 1;	// fail to connect in wait time
            }
        } else {
            DBG_MTX(m_mtx, "connect with " << m_host_address.to_string() << " on " << get_server_port() << " port success");
            break;
        }
        attempts--;
    } while(true);
    return 0;
}

bool c_TCPasync::is_connected() {
    return m_server_socket.is_open();	// TODO
}

void c_TCPasync::add_cmd(c_TCPcommand &cmd) {
	protocol type = cmd.get_type();
	auto cmd_it = find_cmd(type);
    if(cmd_it == m_available_cmd.end()) {
        DBG_MTX(m_mtx, "push_back [" << static_cast<int>(cmd.get_type()) << "] cmd");
		m_available_cmd.push_back(std::ref(cmd));
	} else {
		DBG_MTX(m_mtx, "can't add cmd[" << static_cast<int>(type) << " that already is available");
	}
}

void c_TCPasync::send_cmd_request(protocol type) {
	auto cmd = find_cmd(type);
	if(cmd == m_available_cmd.end()) {
		throw std::invalid_argument("Protocol not allowed/added in this connection");
	};
	cmd->get().send_request(m_server_socket);
}

void c_TCPasync::send_cmd_response(protocol type, const std::string &packet) {
    auto cmd = find_cmd(type);
    cmd->get().set_response(packet);
    cmd->get().send_response(m_server_socket);
}

unsigned short c_TCPasync::get_server_port() {
	unsigned short server_port = m_server_endpoint.port();
	return server_port;
}
unsigned short c_TCPasync::get_local_port() {
	return m_local_port;
}

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_TCPasync::create_server() {
    while (m_io_service.stopped() && !m_stop_flag) {
        std::this_thread::yield();
    }
    if (m_stop_flag) {
        DBG_MTX(m_mtx, "stop flag, return");
        return;
    }
    assert(m_io_service.stopped() == false);
    m_acceptor.async_accept(m_local_socket,
                            [this](boost::system::error_code ec) {
                                DBG_MTX(m_mtx,"async lambda"); // dbg
                                if(!ec) {
                                    DBG_MTX(m_mtx,"do read start"); // dbg
                                    this->server_read(std::move(m_local_socket));
                                } else {
                                    DBG_MTX(m_mtx,"EC = " << ec);
                                    return;
                                }
                                this->create_server();
                            });
}

void c_TCPasync::server_read(ip::tcp::socket &&socket_) {
    assert(socket_.is_open());
    boost::system::error_code ec;
    unsigned short prototype_size = sizeof(uint16_t);

    ip::tcp::socket socket(std::move(socket_));

    while (!ec && !m_stop_flag) {
        assert(socket.is_open());
        protocol type = protocol::empty;
		socket.read_some(buffer(&type, prototype_size), ec);
		if(type == protocol::empty) {
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

std::vector<std::reference_wrapper<c_TCPcommand>>::iterator c_TCPasync::find_cmd(protocol type) {
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

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

c_TCPasync::~c_TCPasync() {
	m_server_socket.close();
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
