#ifndef RPC_HPP
#define RPC_HPP

#include "../libs0.hpp"
#include "c_tcp_asio_node.hpp"

namespace rpc {

void send_tcp_msg(const std::string &msg, const std::string addr = "127.0.0.1", int port = 9040);
void rpc_demo ();

} // namespace rpc

#endif // RPC_HPP
