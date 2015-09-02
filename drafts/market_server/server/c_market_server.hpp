#ifndef SERVER_C_SERVER_H
#define SERVER_C_SERVER_H

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <list>
#include "../hash_table/hash_table.hpp"
#include "../market/c_market.hpp"
#include "../crypto/c_crypto.hpp"

using boost::asio::ip::udp;
using std::cout;
using std::string;
using std::cerr;
using std::atoi;
using std::to_string;
using std::unordered_map;
using std::list;

class c_market_server {
private:
	const unsigned short port;
	const size_t max_length = 4096;

	c_market market;

	void work (boost::asio::io_service &);

	string execute_command (char *, size_t);

	string get_command (char *, size_t, size_t);

	string command_best_buy_offer ();

	string command_best_sell_offer ();

	string command_sell (char *, size_t);

	string command_buy (char *, size_t);

	string command_register (char *, size_t);

	string command_help ();

	string command_login (char *, size_t);

	string command_logout (char *, size_t);

	string command_delete_account (char *, size_t);

	string command_status (char *, size_t);

	c_crypto<2048> crypto;

	c_crypto<2048>::long_type get_sign (char *&, size_t &);

	bool verify_user (char *&, size_t &);

	public_key<c_crypto<2048>::long_type> get_public_key (char *, size_t);

	unordered_map<string, string> users;

	unordered_map<string, public_key<c_crypto<2048>::long_type>> logged_on_users;

public:
	c_market_server (unsigned short port);

	void start ();
};

#endif //SERVER_C_SERVER_H
