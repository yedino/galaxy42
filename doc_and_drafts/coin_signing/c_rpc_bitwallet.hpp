#ifndef C_RPC_BITWALLET_H
#define C_RPC_BITWALLET_H

#include <bitcoinapi/bitcoinapi.h>
#include <memory>
#include <iostream>

class c_rpc_bitwallet
{
public:
    c_rpc_bitwallet();
    bool is_set();
    void set_wallet (std::string username, std::string password, std::string address,int port);
    double get_balance();
    std::string get_account_address(const std::string& account_name = "");
    std::string sendfrom(const std::string& fromaccount, const std::string& tocoinaddress, double amount);

private:
    std::unique_ptr<BitcoinAPI> m_rpc_api;
    bool set = false;
    std::string m_rpc_username;
    std::string m_rpc_password;
    std::string m_rpc_address;
    int m_rpc_port;
};

#endif // C_RPC_BITWALLET_H
