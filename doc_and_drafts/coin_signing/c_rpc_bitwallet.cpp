#include "c_rpc_bitwallet.hpp"

c_rpc_bitwallet::c_rpc_bitwallet() : set(false), m_rpc_api(nullptr)
{ }

bool c_rpc_bitwallet::is_set() {
    return set;
}

void c_rpc_bitwallet::set_wallet(const std::string &username, const std::string &password, const std::string &address,int port) {

    m_rpc_username = username;
    m_rpc_password = password;
    m_rpc_address = address;
    m_rpc_port = port;
    m_rpc_api.reset(new BitcoinAPI(m_rpc_username, m_rpc_password, m_rpc_address, m_rpc_port));

    if(m_rpc_api && m_rpc_api->IsInit()) {
        set = true;
    }
}

double c_rpc_bitwallet::get_balance() {
  try {
    double balance = m_rpc_api->getbalance();
    std::cout << balance << std::endl;
    return balance;
  } catch(BitcoinException &btc_ec) {
        std::cerr << __func__ << ": " << btc_ec.getCode() << ": " << btc_ec.getMessage() << std::endl;
  }
}

std::string c_rpc_bitwallet::get_account_address(const std::string& account_name) {
    std::string address = m_rpc_api->getaccountaddress(account_name);
    return address;
}

std::string c_rpc_bitwallet::sendfrom(const std::string& fromaccount, const std::string& tocoinaddress, double amount) {
    std::string txid = m_rpc_api->sendfrom(fromaccount, tocoinaddress, amount);
    return txid;
}
