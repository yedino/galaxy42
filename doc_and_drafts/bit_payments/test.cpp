#include "c_bit_payments.hpp"

////////////////////////////////////////////////////////////////////////////////////
// test use :
 int main(int argc, char **argv) {

    // look at .bitcoin/bitcoin.conf for usr, pass itc.
    // be sure you have bitcoind running before start bitcoinapi prog

    std::string username = "nmc_admin";
    std::string password = "dontworrybehappy4$";
    std::string address = "127.0.0.1";
    int port = 8336;

  try {
    /* Constructor to connect to the bitcoin daemon */
    c_bit_payments test_wallet(username, password, address, port);
   // test_wallet.show_general_info();
    std::string res = test_wallet.get_account_address("bitanti_acc");
    std::cout << res << std::endl;
    int confirmations = 0; bool include_empty = true;
    test_wallet.show_status(confirmations, include_empty);

//    std::string addr_from;
//    std::string addr_to;
//    double send_amount = 0.007;

//    std::cout << "Address \"from\" : " << (addr_from = test_wallet.get_account_address("from")) << std::endl;
//    std::cout << "Address \"to\" : " << (addr_to = test_wallet.get_account_address("to")) << std::endl;

//    std::cout << "New transaction : " << test_wallet.sendfrom("from",addr_to, send_amount) << std::endl;

//    test_wallet.show_status(confirmations,include_empty);
  } catch (BitcoinException &btc_ec) {
        std::cout << btc_ec.what() << std::endl;
        std::cout << btc_ec.getCode() << ": " << btc_ec.getMessage() << std::endl;
  } catch (...) {
        std::cout << "unexpected error" << std::endl;
        return 1;
  }
 	return 0;
}
