#include "c_netuser.hpp"
#include <utility>

c_netuser::c_netuser(const std::string &username,
                     unsigned short local_port) :
                                                  c_user(username),
                                                  m_TCPasync(local_port),
                                                  m_stop_thread(false),
                                                  m_thread([this]() { return check_inboxes(); }) {
    set_commands();
}

c_netuser::c_netuser(c_user &&user,
                     unsigned short local_port) :
                                                  c_user(std::move(user)),
                                                  m_TCPasync(local_port),
                                                  m_stop_thread(false),
                                                  m_thread([this]() { return check_inboxes(); }) {
    set_commands();
}

void c_netuser::set_commands () {
    std::string pubkey = std::string(reinterpret_cast<const char*>(get_public_key().c_str()),get_public_key().size());
    std::shared_ptr<c_TCPcommand> s_pubkey_cmd(new c_TCPcommand(packet_type::public_key,pubkey));
    std::shared_ptr<c_TCPcommand> s_token_cmd(new c_TCPcommand(packet_type::token_send,pubkey));
    std::shared_ptr<c_TCPcommand> s_contract_cmd(new c_TCPcommand(packet_type::contract,pubkey));
    m_TCPcommands.emplace(std::make_pair(packet_type::public_key,s_pubkey_cmd));
    m_TCPcommands.emplace(std::make_pair(packet_type::token_send,s_token_cmd));
    m_TCPcommands.emplace(std::make_pair(packet_type::contract,s_contract_cmd));
    for (auto &cmd : m_TCPcommands) {
        m_TCPasync.add_cmd(*cmd.second);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////// networking

void c_netuser::send_token_bynet(const std::string &host, unsigned short server_port) {

    ed_key host_pubkey = m_TCPasync.connect(host, server_port)->get_remote_pubkey();

    std::string packet = get_token_packet(serialization::Json, host_pubkey);
    if (packet == "fail") {
        std::cout << "stop sending token -- empty wallet" << std::endl;
        return;
    }

    m_TCPasync.send_cmd_response(packet_type::token_send, host, server_port, packet);
}

unsigned int c_netuser::get_port() {
    return m_TCPasync.get_port();
}

c_netuser::~c_netuser() {
    m_stop_thread = true;
    m_thread.join();
}

void c_netuser::send_contract(const ed_key &recipient, c_contract contract) {
    std::cout << "Try to send contract to [" << recipient << "]" << std::endl;
    std::string contract_data = contract.to_packet();

    auto recipient_connection = m_TCPasync.find_by_pubkey(recipient);
    if(recipient_connection != nullptr) {
        std::cout << "Found connection: [" << recipient_connection->get_remote_endpoint().port()
                  << "," <<  recipient_connection->get_remote_endpoint().address() << "]" << std::endl;

        auto contract_cmd = m_TCPcommands.find(packet_type::contract);
        if(contract_cmd == m_TCPcommands.end()) {
            std::cout << "can't find protocol: return" << std::endl;
            return;
        }
        contract_cmd->second->send_response(recipient_connection->get_socket());

        // CJDNS // TODO end
        std::string a("v7zrh17f30b1g1fll8kqd6qb6vvbj1d2ldzgkwbg8wmvrw88z020.k");
        std::string command = "./tools/cexec 'InterfaceController_adminSetUpLimitPeer(pubkey=\"" + a +  "\", limitUp=300)'";
        std::cout << "Setting cjdns limitiation :" << command << std::endl;
        system(command.c_str());
    } else {
        //m_TCPasync.send_cmd_response(packet_type::contract, host, server_port, contract_data);
        //TODO looking for recipient address in network
    }
}

void c_netuser::check_inboxes () {
    while(!m_stop_thread) {
        recieve_coin();
        recieve_contract();
        if(!m_contracts_to_send.empty()) {
            std::pair<ed_key, c_contract> recipient_contract(m_contracts_to_send.pop());
            send_contract(recipient_contract.first,recipient_contract.second);
        }
        std::this_thread::yield();
    }
}

void c_netuser::recieve_coin() {
    auto cmd_it = m_TCPcommands.find(packet_type::token_send);
    if(cmd_it == m_TCPcommands.end()) {
        //std::cout << "can't find protocol: return" << std::endl;
        return;
    }
    auto cmd = cmd_it->second;

    std::string tok_str;
    if(cmd->has_message()) {
        std::cout << "Pop new coin" << std::endl;
        tok_str = cmd->pop_message();
        recieve_from_packet(tok_str);
    } else {
            //std::cout << "No waiting coin" << std::endl;
    }
}

void c_netuser::recieve_contract() {
    auto cmd_it = m_TCPcommands.find(packet_type::contract);
    if(cmd_it == m_TCPcommands.end()) {
        //std::cout << "can't find protocol: return" << std::endl;
        return;
    }
    auto cmd = cmd_it->second;

    std::string handle;
    if(cmd->has_message()) {
        std::cout << "Pop new contract" << std::endl;
        handle = cmd->pop_message();
        m_signed_contracts.push_back(c_contract(handle));
    } else {
            //std::cout << "No waiting contract" << std::endl;
    }
}
