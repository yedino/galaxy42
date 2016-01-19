#ifndef SERVER_C_SERVER_MSG_PARSER_H
#define SERVER_C_SERVER_MSG_PARSER_H
#include <string>
#include <iostream>
#include "../../crypto_ops/crypto/c_crypto_ed25519.hpp"
#include "c_market_user.hpp"
#include <boost/asio.hpp>

using std::string;
using std::cout;

struct c_message {
		string command;
		string Line[5];
		string sender_nickname;
		unsigned char Crypto_data[64];
		size_t org_msg_start;
		bool is_ok;

		c_message () : command(""), Line({}), sender_nickname(""), Crypto_data {}, org_msg_start(0), is_ok(true) { }

		void print () const {
			size_t max;
			string crypto_data_type;
			if (command == "register")
				max = 64, crypto_data_type = "public key";
			else
				max = 32, crypto_data_type = "signature";

			cout << "is_ok: ";
			cout << is_ok << '\n';
			cout << "command: ";
			cout << command << '\n';
			cout << "sender nickname: ";
			cout << sender_nickname << '\n';
			cout << "other arguments: ";
			for (size_t i = 0; i < 5 && !Line[i].empty(); ++i)
				cout << Line[i] << ' ';

			cout << '\n' << crypto_data_type << ": ";
			for (size_t i = 0; i < max; ++i)
				cout << (int)Crypto_data[i] << ' ';

			cout << std::endl;
		}
};

class c_server_msg_parser {
private:
		size_t parse_crypto_data (const char *, c_message &, size_t);

public:
		c_message parse (const char *, size_t);
};


#endif //SERVER_C_SERVER_MSG_PARSER_H
