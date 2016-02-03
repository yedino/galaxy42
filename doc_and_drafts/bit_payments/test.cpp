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
    c_bit_payments test_wallet_sender(username, password, address, port);
    c_bit_payments test_wallet_receiver(username, password, address, port);

    test_wallet_sender.show_general_info();
    std::string client_acc_name = "client";

    std::string sender_address = test_wallet_sender.get_account_address(client_acc_name);
    std::string reciever_address = test_wallet_receiver.get_account_address("payment_destination");
    std::cout << "client address:" << sender_address << std::endl;
    std::cout << "payment_destination address: " << reciever_address << std::endl;

    int confirmations = 0; bool include_empty = true;
    test_wallet_sender.show_status(confirmations, include_empty);


    std::string destination_addr = reciever_address;
    double send_amount = 0.01; 		// antinet_cost

    std::cout << "New transaction : " << test_wallet_sender.sendfrom(client_acc_name,destination_addr, send_amount) << std::endl;

    test_wallet_sender.show_status(confirmations,include_empty);


  } catch (BitcoinException &btc_ec) {
        std::cout << btc_ec.what() << std::endl;
        std::cout << btc_ec.getCode() << ": " << btc_ec.getMessage() << std::endl;
  } catch (...) {
        std::cout << "unexpected error" << std::endl;
        return 1;
  }
 	return 0;
}
