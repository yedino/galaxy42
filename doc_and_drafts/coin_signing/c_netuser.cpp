#include "c_netuser.hpp"
#include <iostream>

const unsigned request_type_size = 2;

c_netuser::c_netuser(std::string& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service),
                                              m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v4(),server_port)),
                                              m_stop_flag(false)
{
    threads_maker(2);
    create_server();
}

c_netuser::c_netuser(std::string&& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service),
                                              m_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v4(),server_port)),
                                              m_stop_flag(false)
{
    threads_maker(2);
    create_server();
}

void c_netuser::send_pubkey_request(const std::string &ip_address) {

    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip_address, ec );
    if(ec) { ///< boost error - not needed
        throw std::runtime_error("bad ip");
    }
    if(!addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " is not valid IPv4 address";
        throw std::exception(std::invalid_argument(msg));
    }
    ip::tcp::endpoint server_endpoint(addr, server_port);

    char pubkey_send_req[2] = {'p','k'};
    uint32_t packet_size = ed25519_sizes::pub_key_size;
    std::string packet = get_public_key();

    client_socket.connect(server_endpoint,ec);
    if(ec) {
        DBG_MTX(dbg_mtx,"EC = " << ec);
        throw std::runtime_error("get_pubkey_for_transactions -- fail to connect");
    }

    client_socket.write_some(boost::asio::buffer(pubkey_send_req, request_type_size),ec);

    DBG_MTX(dbg_mtx,"send public key size" << "[" << packet_size << "]");
    client_socket.write_some(boost::asio::buffer(&packet_size, 4),ec);

    DBG_MTX(dbg_mtx,"send public key data" << "[" << packet << "]");
    client_socket.write_some(boost::asio::buffer(packet.c_str(), packet_size),ec);
    DBG_MTX(dbg_mtx,"end of sending public key");
    client_socket.close();
}

void c_netuser::send_token_bynet(const std::string &ip_address, const std::string &reciever_pubkey) {

    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip_address, ec );
    if(ec) { ///< boost error - not needed
        throw std::runtime_error("bad ip");
    }
    if(!addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " is not valid IPv4 address";
        throw std::exception(std::invalid_argument(msg));
    }
    ip::tcp::endpoint server_endpoint(addr, server_port);


    std::string packet = get_token_packet(reciever_pubkey);
    if(packet == "fail") {
        DBG_MTX(dbg_mtx,"stop sending token -- empty wallet");
        return;
    }
    uint32_t packet_size = packet.size();
    char tok_send_req[2] = {'$','t'};


    client_socket.connect(server_endpoint,ec);
    if(ec) {
        DBG_MTX(dbg_mtx,"EC = " << ec);
        throw std::runtime_error("send_token_bynet -- fail to connect");
    }

    client_socket.write_some(boost::asio::buffer(tok_send_req, request_type_size),ec);

    DBG_MTX(dbg_mtx,"send coin size" << "[" << packet_size << "]");
    client_socket.write_some(boost::asio::buffer(&packet_size, 4),ec);

    DBG_MTX(dbg_mtx,"send coin data" << "[" << packet << "]");
    client_socket.write_some(boost::asio::buffer(packet.c_str(), packet_size),ec);
    DBG_MTX(dbg_mtx,"end of sending token");
    client_socket.close();
}

void c_netuser::create_server() {
    DBG_MTX(dbg_mtx,"accept on port " << server_port);
    //ip::tcp::acceptor my_acceptor(m_io_service, ip::tcp::endpoint(ip::tcp::v4(),server_port));
    m_acceptor.async_accept(server_socket,
                            [this](boost::system::error_code ec) {
                                DBG_MTX(dbg_mtx,"async lambda");
                                if(!ec) {
                                    DBG_MTX(dbg_mtx,"do read start");
                                    this->do_read(std::move(server_socket));
                                } else {
                                    DBG_MTX(dbg_mtx,"EC = " << ec);
                                }
                                this->create_server();
                            });
    DBG_MTX(dbg_mtx,"end of async accept");
}

void c_netuser::do_read(ip::tcp::socket socket_) {
    DBG_MTX(dbg_mtx,"do read");
    boost::system::error_code ec;
    char msg_type[2];						// first 2 bytes always is type of message
    memset(msg_type,0,2);

    ip::tcp::socket local_socket(std::move(socket_));
    size_t recieved_bytes;
    recieved_bytes = local_socket.read_some(buffer(msg_type,2),ec);
    assert(recieved_bytes == 2);

    if(strcmp(msg_type,"pk") != 0) {
        read_pubkey(std::move(local_socket));
    } else if(strcmp(msg_type,"$t") != 0) {
        read_token(std::move(local_socket));
    } else {
        DBG_MTX(dbg_mtx,"Cant't recognize type of msg: " << msg_type);
    }
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
        m_threads.emplace_back([this](){
			while (!m_stop_flag)
				this->m_io_service.run();
		});
    }
}
