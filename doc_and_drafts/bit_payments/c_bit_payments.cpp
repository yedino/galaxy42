#include "c_bit_payments.hpp"

c_bit_payments::c_bit_payments(std::string username, std::string password, std::string address, int port) :
	btc (new BitcoinAPI(username, password, address, port)) {
}

c_bit_payments::~c_bit_payments() {
}

std::string c_bit_payments::get_account_address(const std::string& account_name) {
	std::string address = btc->getaccountaddress(account_name);
	return address;
}
std::string c_bit_payments::sendfrom(const std::string& fromaccount, const std::string& tobitcoinaddress, double amount) {
	std::string txid = btc->sendfrom(fromaccount, tobitcoinaddress, amount);
	return txid;
}

void c_bit_payments::show_general_info() {

	getinfo_t info = btc->getinfo();
	std::cout << " * * * * * * * * * * * * * * GENERAL INFO  * * * * * * * * * * * * * * *"
			<< "\nversion : " << info.version
			<< "\nprotocolversion : " << info.protocolversion
			<< "\nwalletversion : " << info.walletversion
			<< "\nbalance : " << info.balance
			<< "\nblocks : " << info.blocks
			<< "\ntimeoffset : " << info.timeoffset
			<< "\nconnections : " << info.connections
			<< "\nproxy : " << info.proxy
			<< "\ndifficulty : " << info.proxy
			<< "\ntestnet : " << info.testnet
			<< "\nkeypoololdest : " << info.keypoololdest
			<< "\nkeypoolsize : " << info.keypoololdest
			<< "\npaytxfee : " << info.paytxfee
			<< "\nunlocked_until : " << info.unlocked_until
			<< "\nerrors : " << info.errors << std::endl;
			
	std::cout << "Total wallet balance : " << btc->getbalance()
			<< "\nUnconfirmed balance : " << btc->getunconfirmedbalance() << std::endl;
}

void c_bit_payments::show_status(int confirmations, bool include_empty) {

	std::cout << " * * * * * * * * * * * * INFO by ADDRESSES * * * * * * * * * * * * * * *" << std::endl;
	
	std::vector<addressinfo_t> v_adrrinfo = btc->listreceivedbyaddress(confirmations, include_empty);
	std::cout << "address : receivedbyaddress" << "\n\ttransactions" << std::endl;
	std::cout << "------------------------------------------------------------------------" << std::endl;
    for(auto addr : v_adrrinfo) {
        std::cout << addr.address << " : " <<  btc->getreceivedbyaddress(addr.address,confirmations) << std::endl;
        for(auto txids : addr.txids) {
            std::cout << '\t'  << txids << std::endl;
			gettransaction_t txid_t = btc->gettransaction(txids);
			std::cout << "\tamount : " << txid_t.amount << ", confirmation : " << txid_t.confirmations << std::endl;
			for(auto tx_details : txid_t.details) {
				std::cout << "\tfrom account : \"" << tx_details.account
						<< "\"\n\t to address : " << tx_details.address
						<< "\n\tamount : " << tx_details.amount << ", category : " << tx_details.category << std::endl;
			}
			std::cout << "\twallet conflicts: [";
			if(!txid_t.walletconflicts.empty()) {
			std::cout << txid_t.walletconflicts.at(0);
				for(size_t i = 1; i < txid_t.walletconflicts.size(); ++i) {
					std::cout << ", " << txid_t.walletconflicts.at(i);
				}
			}
			std::cout << ']' << std::endl;
		}
		std::cout << "------------------------------------------------------------------------" << std::endl;
	}

   std::cout << " * * * * * * * * * * * * INFO by ACCOUNTS * * * * * * * * * * * * * * *" << std::endl;

    std::map<std::string, double> list_of_accounts = btc->listaccounts(confirmations);
    for(auto acc : list_of_accounts) {
        std::string account_name = acc.first;	
        std::cout << "account : " << ((account_name == "") ? "noname" : account_name) << std::endl;
        std::cout << "balance : " << btc->getbalance(account_name) << std::endl;
        std::vector<std::string> acc_addresses = btc->getaddressesbyaccount(account_name);
        std::cout << "address : " << std::endl;
        for(auto addr : acc_addresses) {
            std::cout << '\t' << addr << std::endl;
        }
        std::cout << "--------------------------------------------------" << std::endl;
    }
}
