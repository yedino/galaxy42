#ifndef SERVER_C_SERVER_H
#define SERVER_C_SERVER_H

#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include "c_market.hpp"
#include "c_server_msg_parser.hpp"
#include "c_market_user.hpp"
#include "../../crypto_ops/crypto/sha_src/sha512.hpp"
#include "c_users_database.hpp"

using boost::asio::ip::udp;
using std::cout;
using std::string;
using std::cerr;
using std::atoi;
using std::to_string;
using std::unordered_map;
using std::unordered_set;
using std::vector;

class c_market_server {
private:
		const unsigned short port;

		const size_t max_length = 4096;

		string answer;

		string demo_request;

		c_market market;

		c_crypto_ed25519 crypto_api;

		c_server_msg_parser parser;

		c_users_database users;

		void work (boost::asio::io_service &);

		void execute_command (const c_message &, const c_market_user &); // TODO make it more smart, bro

		void command_best_buy_offer ();

		void command_best_sell_offer ();

		void command_sell (const c_message &, const c_market_user &);

		void command_buy (const c_message &, const c_market_user &);

		void command_register (const c_message &);

		void command_help ();

		void command_delete_account (const c_message &, const c_market_user &);

		void command_status (const c_message &, const c_market_user &); // TODO IMPLEMENT THIS

		bool verify_signature (const string &, const c_message &, const c_market_user &);

		void init_execute_command (const char *, size_t, const boost::asio::ip::address &);

		void command_change_public_key (const c_message &message, const c_market_user &user); // TODO IMPLEMENTS THIS

public:
		c_market_server (unsigned short port); // TODO IMPLEMENT THIS + SIGN SERVER'S MSGS

		void start ();

		void start_demo ();
};

#endif //SERVER_C_SERVER_H
