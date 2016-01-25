#include "c_netuser.hpp"

c_netuser::c_netuser(std::string& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service) {
}

c_netuser::c_netuser(std::string&& username) : c_user(username),
                                              client_socket(m_io_service),
                                              server_socket(m_io_service) {
}



void c_netuser::send_token_bynet(const std::string &ip_address) {
    
    boost::system::error_code ec;
    ip::address addr = ip::address::from_string(ip_address, ec );
    if(ec) { ///< boost error - not needed
        //throw std::exception(ec.message());
    }
    if(!addr.is_v4()) {
        std::string msg = addr.to_string();
        msg += " is not valid IPv4 address";
        throw std::exception(std::invalid_argument(msg));
    }
    ip::tcp::endpoint server_endpoint(addr, server_port);

    client_socket.connect(server_endpoint,ec);

}

c_netuser::~c_netuser() {

    m_io_service.stop();
    for(auto &t : m_threads){
        t.join();
    }
}

void c_netuser::threads_maker(unsigned num) {

    m_threads.reserve(num);
    for(int i = 0; i < num; ++i) {
        m_threads.emplace_back([this](){this->m_io_service.run();});
    }
}
