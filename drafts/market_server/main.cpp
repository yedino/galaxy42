#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
#include "c_market_server.hpp"
#include "c_server_msg_parser.hpp"
#include <vector>
#include "c_crypto_ed25519.hpp"

using namespace std;

void parser_test () {
	c_server_msg_parser parser;

	vector<string> commands = {"REGISTER", "SELL", "BUY", "BEST_SELL_OFFER", "BEST_BUY_OFFER", "STATUS", "DELETE_ACCOUNT",
		"CHANGE_PUBLIC_KEY", "HELP"};
	string signature = "ed728b7ed933b6a2e7869696a6b60113aa2d93a43eec7824bece841c51b67401df01e419";
	string public_key = "ed728b7ed933b6a2e7869696a6b60113aa2d93a43eec7824bece841c51b67401df01e498cd68b3951db278152a6cb606a0b433af74b8f3aa20cdef69dfefd3c40b";

	string string_data[9];
	string_data[0] = public_key + ":register:test001";
	string_data[1] = signature + ":sell 144 25:test001";
	string_data[2] = signature + ":buy 144 25:test001";
	string_data[3] = "best_sell_offer";
	string_data[4] = "best_buy_offer";
	string_data[5] = signature + ":status:test001";
	string_data[6] = signature + ":delete_account:test001";
	string_data[7] = signature + ":change_public_key " + public_key + ":test001";
	string_data[8] = "help";

	for (size_t i = 0; i < 9; ++i) {
		cout << "--------------" << commands.at(i) << "--------------\n";
		auto msg = parser.parse(string_data[i].c_str(), string_data[i].size());
		cout << "original msg: ";
		cout << string(string_data[i].c_str() + msg.org_msg_start, string_data[i].size() - msg.org_msg_start) << '\n';
		msg.print();
		cout << '\n';
		cout.flush();
	}
}

void demo_test () { // TODO implements this
	c_crypto_ed25519 crypto_api;
	crypto_api.generate_key();

	c_market_server server_api(55555);
}

int main (int argc, char *argv[]) {
	//	c_market_server server(55555);
	//	server.start();

	//	parser_test();
	ios_base::sync_with_stdio(false);

	demo_test();
	return 0;
}