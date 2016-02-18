#include "c_netuser.hpp"
#include <iostream>

const unsigned request_type_size = 2;

c_netuser::c_netuser(std::string &username, int port) : c_user(username),
                                              server_port(port),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service),
                                              m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v4(),server_port)),
                                              m_stop_flag(false)
{
    create_server();
    threads_maker(2);
}


ustring c_netuser::get_public_key_resp(ip::tcp::socket &socket_) {
	DBG_MTX(dbg_mtx, "START");
	assert(socket_.is_open());
	boost::system::error_code ec;

	char header[2];
	DBG_MTX(dbg_mtx, "read header");
    size_t pkresp = socket_.read_some(buffer(header, 2), ec);
    DBG_MTX(dbg_mtx, "pk: " << pkresp << ":[" <<header[0] << header[1] << "]");

	DBG_MTX(dbg_mtx, "read public key size");
	char pub_key_size[4];
    uint32_t key_size = 0;
	size_t recieved_bytes = socket_.read_some(buffer(&key_size, 4), ec);
    DBG_MTX(dbg_mtx, "size:" << recieved_bytes << ":[" <<key_size << "]");

    assert(recieved_bytes == 4);

    const std::unique_ptr<unsigned char[]> pub_key_data(new unsigned char[key_size]);

	DBG_MTX(dbg_mtx, "read public key data");
    recieved_bytes = socket_.read_some(buffer(pub_key_data.get(), key_size), ec);
    DBG_MTX(dbg_mtx, "data:" << recieved_bytes << ":[" << ustring(pub_key_data.get(),recieved_bytes) << "]");
    assert(recieved_bytes == key_size);

    ustring pub_key(pub_key_data.get(), key_size);
	DBG_MTX(dbg_mtx, "END");
	return pub_key;
}

void c_netuser::send_public_key_req(ip::tcp::socket &socket_) {
	DBG_MTX(dbg_mtx, "send public key request");
	assert(socket_.is_open());
    boost::system::error_code ec;
    char pubkey_send_req[2] = {'p','k'};
    size_t sendbytes = socket_.write_some(boost::asio::buffer(pubkey_send_req, request_type_size),ec);
    DBG_MTX(dbg_mtx, "pk: " << sendbytes << ":[" << pubkey_send_req[0] <<  pubkey_send_req[1] << "]");

}

void c_netuser::send_public_key_resp(ip::tcp::socket &socket_) {
	DBG_MTX(dbg_mtx, "START");
	assert(socket_.is_open());
	boost::system::error_code ec;
	char header[2] = {'p', 'k'};
	DBG_MTX(dbg_mtx, "send header");
	socket_.write_some(buffer(header, 2), ec);
	//uint32_t packet_size = ed25519_sizes::pub_key_size;
    ustring packet = get_public_key();
	uint32_t packet_size = packet.size();
	// TODO send binary data
	DBG_MTX(dbg_mtx,"send public key size" << "[" << packet_size << "]");
    socket_.write_some(boost::asio::buffer(&packet_size, 4), ec);

    DBG_MTX(dbg_mtx,"send public key data" << "[" << packet << "]");
    socket_.write_some(boost::asio::buffer(packet.c_str(), packet_size), ec);
    DBG_MTX(dbg_mtx,"end of sending public key");
}

void c_netuser::send_coin(ip::tcp::socket &socket_, const std::string &coin_data) {
	DBG_MTX(dbg_mtx, "START");
	assert(socket_.is_open());
    boost::system::error_code ec;
	char header[2] = {'$', 't'};
	socket_.write_some(buffer(header, 2), ec);

	uint32_t coin_data_size = coin_data.size();
	socket_.write_some(buffer(&coin_data_size, 4), ec);

	socket_.write_some(buffer(coin_data), ec);
}

string c_netuser::recv_coin(ip::tcp::socket &socket_) {
	DBG_MTX(dbg_mtx, "START");
	assert(socket_.is_open());
	boost::system::error_code ec;

    size_t recieved_bytes = 0;

	uint32_t coin_size = 0;
	recieved_bytes = socket_.read_some(buffer(&coin_size, 4), ec);
	DBG_MTX(dbg_mtx, "get " << recieved_bytes << " bytes");
	assert(recieved_bytes == 4);

	const std::unique_ptr<char[]> coin_data(new char[coin_size]);

	recieved_bytes = socket_.read_some(buffer(coin_data.get(), coin_size), ec);
	assert(recieved_bytes == coin_size);
	return std::string(coin_data.get(), coin_size);
}


void c_netuser::send_token_bynet(const std::string &ip_address, int port) {
	DBG_MTX(dbg_mtx, "START");
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip_address, ec);
    if(ec) { ///< boost error - not needed
        throw std::runtime_error("bad ip");
    }
    if (!addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " is not valid IPv4 address";
        throw std::invalid_argument(msg);
    }
    ip::tcp::endpoint server_endpoint(addr, port);

	ip::tcp::socket socket_(m_io_service);
    socket_.connect(server_endpoint, ec);
    if (ec) {
        DBG_MTX(dbg_mtx,"EC = " << ec);
        throw std::runtime_error("send_token_bynet -- fail to connect");
    }

	DBG_MTX(dbg_mtx, "getting remote public key");
	send_public_key_req(socket_);
    ustring remote_public_key(get_public_key_resp(socket_));
	DBG_MTX(dbg_mtx, "remote public key " << remote_public_key);

    std::string packet = get_token_packet(remote_public_key);
    if (packet == "fail") {
        DBG_MTX(dbg_mtx,"stop sending token -- empty wallet");
        return;
    }

    send_coin(socket_, packet);

    socket_.close();
}

void c_netuser::create_server() {
    DBG_MTX(dbg_mtx,"accept on port " << server_port);
	while (m_io_service.stopped() && !m_stop_flag) {
		//std::cout << "loop " << std::this_thread::get_id() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	if (m_stop_flag) {
		DBG_MTX(dbg_mtx, "stop flag, return");
		return;
	}
	assert(m_io_service.stopped() == false);
    m_acceptor.async_accept(server_socket,
                            [this](boost::system::error_code ec) {
                                DBG_MTX(dbg_mtx,"async lambda");
                                if(!ec) {
                                    DBG_MTX(dbg_mtx,"do read start");
                                    this->server_read(std::move(server_socket));
                                } else {
                                    DBG_MTX(dbg_mtx,"EC = " << ec);
                                }
                                this->create_server();
                            });
    DBG_MTX(dbg_mtx,"end of async accept");
}

void c_netuser::server_read(ip::tcp::socket socket_) {
	DBG_MTX(dbg_mtx, "START");
	assert(socket_.is_open());
    boost::system::error_code ec;
    DBG_MTX(dbg_mtx,"server read");
	while (!ec && !m_stop_flag) {
		char header[2] = {0, 0};
		socket_.read_some(buffer(header, 2), ec);
		if (header[0] == 'p' && header[1] == 'k') {
			send_public_key_resp(socket_);
		}
		else if (header[0] == '$' && header[1] == 't') {
			std::string coin_data = recv_coin(socket_);
			recieve_from_packet(coin_data);
		}
	}
	socket_.close();
}

void c_netuser::read_pubkey(ip::tcp::socket socket_) {

    DBG_MTX(dbg_mtx,"read public key");
    boost::system::error_code ec;
    char pk_s[4];						// first 4 bytes always is lenght of packet
    memset(pk_s,0,4);

    ip::tcp::socket local_socket(std::move(socket_));
    size_t recieved_bytes;
    recieved_bytes = local_socket.read_some(buffer(pk_s,4),ec);
    assert(recieved_bytes == 4);
    uint32_t* pk_size = reinterpret_cast<uint32_t*>(pk_s);
    DBG_MTX(dbg_mtx,"public key size: " << *pk_size);

    std::string pk;
    recieved_bytes = 0;
    while(recieved_bytes < *pk_size) {
        size_t loop_data_size = local_socket.read_some(buffer(data_,max_length),ec);
        DBG_MTX(dbg_mtx,"rec " << loop_data_size << " bytes");
        pk.assign(data_,loop_data_size);

        recieved_bytes += loop_data_size;
    }
    DBG_MTX(dbg_mtx,"got public key: " << pk);
}

void c_netuser::read_token(ip::tcp::socket socket_) {

    DBG_MTX(dbg_mtx,"read token");
    boost::system::error_code ec;
    char tok_s[4];						// first 4 bytes always is lenght of packet
    memset(tok_s,0,4);

    ip::tcp::socket local_socket(std::move(socket_));
    size_t recieved_bytes;
    recieved_bytes = local_socket.read_some(buffer(tok_s,4),ec);
    assert(recieved_bytes == 4);
    uint32_t* tok_size = reinterpret_cast<uint32_t*>(tok_s);
    DBG_MTX(dbg_mtx,"token size: " << *tok_size);

    std::string packet;
    recieved_bytes = 0;
    while(recieved_bytes < *tok_size) {
        size_t loop_data_size = local_socket.read_some(buffer(data_,max_length),ec);
        DBG_MTX(dbg_mtx,"rec " << loop_data_size << " bytes");
        packet.assign(data_,loop_data_size);

        recieved_bytes += loop_data_size;
    }
    recieve_from_packet(packet);
}

c_netuser::~c_netuser() {
	m_stop_flag = true;
    m_io_service.stop();
    for(auto &t : m_threads){
        t.join();
    }
}

void c_netuser::threads_maker(unsigned num) {
    m_threads.reserve(num);
    for(int i = 0; i < num; ++i) {
        DBG_MTX(dbg_mtx,"make " << i << " thread");
        m_threads.emplace_back([this](){
			while (!m_stop_flag)
			{
				this->m_io_service.reset();
				this->m_io_service.run();
			}
			DBG_MTX(dbg_mtx, "end of thread");
		});
    }
}
