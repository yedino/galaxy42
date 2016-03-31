#include "c_netuser.hpp"

const unsigned request_type_size = 2;

c_netuser::c_netuser(const std::string &username,
                     const std::string &host,
                     unsigned short server_port,
                     unsigned short local_port) :
                                                  c_user(username),
                                                  m_TCPasync(host, server_port, local_port),
                                                  m_stop_thread(true),
                                                  m_thread([this]() { return check_inboxes(); }) {

    std::string pubkey = std::string(reinterpret_cast<const char*>(get_public_key().c_str()),get_public_key().size());
    m_TCPcommands.reserve(5);
    m_TCPcommands.push_back(c_TCPcommand(protocol::public_key,pubkey));
    m_TCPcommands.push_back(c_TCPcommand(protocol::token_send));
    m_TCPcommands.push_back(c_TCPcommand(protocol::contract));
    for(auto &cmd : m_TCPcommands) {
        m_TCPasync.add_cmd(cmd);
    }
}

c_netuser::c_netuser(c_user &&user,
                     const std::string &host,
                     unsigned short server_port,
                     unsigned short local_port) :
                                                  c_user(std::move(user)),
                                                  m_TCPasync(host, server_port, local_port),
                                                  m_stop_thread(true),
                                                  m_thread([this]() { return check_inboxes(); }) {

    std::string pubkey = std::string(reinterpret_cast<const char*>(get_public_key().c_str()),get_public_key().size());
    m_TCPcommands.reserve(5);
    m_TCPcommands.push_back(c_TCPcommand(protocol::public_key,pubkey));
    m_TCPcommands.push_back(c_TCPcommand(protocol::token_send));
    m_TCPcommands.push_back(c_TCPcommand(protocol::contract));
    for(auto &cmd : m_TCPcommands) {
        m_TCPasync.add_cmd(cmd);
    }
}

void c_netuser::set_target(const std::string &host, unsigned short server_port) {
    m_TCPasync.set_target(host, server_port);
}

unsigned short c_netuser::get_server_port() {
    return m_TCPasync.get_server_port();
}

unsigned short c_netuser::get_local_port() {
    return m_TCPasync.get_local_port();
}

//////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_netuser::send_token_bynet(){
    m_TCPasync.send_cmd_request(protocol::public_key);

    auto &cmd = m_TCPcommands.at(0);

    std::string handle;
    int attempts = 5;
    do {
        if(cmd.has_message()) {
            handle = cmd.pop_message();
            break;
        } else {
            std::cout << "Attempt: " << attempts << " waiting for response" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        if(attempts == 0) {
            throw std::runtime_error("Fail to get handshake response in wait time");
        }
        attempts--;
    } while(true);

    ed_key host_pubkey(reinterpret_cast<const unsigned char*>(handle.c_str()),handle.size());	// TODO

    std::string packet = get_token_packet(serialization::Json, host_pubkey);
    if (packet == "fail") {
        std::cout << "stop sending token -- empty wallet" << std::endl;
        return;
    }

    m_TCPasync.send_cmd_response(protocol::token_send, packet);
}

void c_netuser::send_token_bynet(const std::string &host, unsigned short server_port) {
    set_target(host, server_port);
    send_token_bynet();
}

c_netuser::~c_netuser() {
    m_stop_thread = false;
    m_thread.join();
}

void c_netuser::send_contract() {

    boost::system::error_code ec;
    if (m_contracts_to_send.empty()) {
         throw std::logic_error("No contracts to send");
    }
    std::string contract_data = m_contracts_to_send.pop().to_packet();
    m_TCPasync.send_cmd_response(protocol::contract, contract_data);

    std::string a("v7zrh17f30b1g1fll8kqd6qb6vvbj1d2ldzgkwbg8wmvrw88z020.k");
    std::string command = "./tools/cexec 'InterfaceController_adminSetUpLimitPeer(pubkey=\"" + a +  "\", limitUp=300)'";
    std::cout << "Setting cjdns limitiation :" << command << std::endl;
    system(command.c_str());
}

void c_netuser::check_inboxes () {
    while(!m_stop_thread) {
        recieve_coin();
        recieve_contract();
        std::this_thread::yield();
    }
}

void c_netuser::recieve_coin() {
    auto &cmd = m_TCPcommands.at(1);
    std::string handle;
    if(cmd.has_message()) {
        std::cout << "Pop new coin" << std::endl;
        handle = cmd.pop_message();
        c_token tok(handle, serialization::Json);
        m_wallet.move_token(std::move(tok));
    } else {
            //std::cout << "No waiting coin" << std::endl;
    }
}

void c_netuser::recieve_contract() {

    auto &cmd = m_TCPcommands.at(2);
    std::string handle;
    if(cmd.has_message()) {
        std::cout << "Pop new contract" << std::endl;
        handle = cmd.pop_message();
        m_signed_contracts.push_back(c_contract(handle));
    } else {
            //std::cout << "No waiting contract" << std::endl;
    }
}
