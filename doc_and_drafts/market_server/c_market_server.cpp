#include "c_market_server.hpp"

c_market_server::c_market_server (unsigned short port) : port(port), market(c_market()) {
	//	crypto.generate_key();
}

void c_market_server::start () {
	try {
		boost::asio::io_service io_service;
		work(io_service);
	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

void c_market_server::work (boost::asio::io_service &io_service) {
	udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
	string answer;
	for (; ;) {
		char data[max_length];
		udp::endpoint sender_endpoint;
		size_t length = sock.receive_from(boost::asio::buffer(data, max_length), sender_endpoint);
		data[length] = 0;

		cerr << "RECIEVED MESSAGE FROM " << sender_endpoint << '\n';
		cout << "message:     ";

		for (size_t i = 0; i < length; ++i)
			cout << data[i];

		cout << '\n';
		cout << "     length of message: " << length << '\n';

		answer.clear();
		init_execute_command(data, length, sender_endpoint.address());
		if (answer.empty())
			answer = "some error occured";

		sock.send_to(boost::asio::buffer(answer.c_str(), answer.size()), sender_endpoint);
	}
}

void c_market_server::init_execute_command (const char *data, size_t length, const boost::asio::ip::address &addr) {
	auto msg = parser.parse(data, length);

	if (!msg.is_ok) {
		answer = "parser error";
		return;
	}

	if (msg.command == "register")
		return command_register(msg);

	else {
		if (!users.check_if_exists(msg.sender_nickname))
			answer = "no such user";

		auto &sender = users.get_user_by_nickname(msg.sender_nickname);
		string org_msg = string(data + msg.org_msg_start, length - msg.org_msg_start);

		if (!verify_signature(org_msg, msg, sender)) {
			answer = "signature verification error";
			return;
		}

		return execute_command(msg, sender);
	}
}

void c_market_server::execute_command (const c_message &msg, const c_market_user &user) {
	if (msg.command == "best_buy_offer") {
		return command_best_buy_offer();

	} else if (msg.command == "best_sell_offer") {
		return command_best_sell_offer();

	} else if (msg.command == "sell") {
		return command_sell(msg, user);

	} else if (msg.command == "buy") {
		return command_buy(msg, user);

	} else if (msg.command == "help") {
		return command_help();

	} else if (msg.command == "delete_account") {
		return command_delete_account(msg, user);

	} else if (msg.command == "status") {
		return command_status(msg, user);

	} else if (msg.command == "moo") {
		answer = "         ^__^\n";
		answer += "         (oo)\\_______\n";
		answer += "         (__)\\       )\\/\\ \n";
		answer += "             ||----w | \n";
		answer += "             ||     ||";

	} else if (msg.command == "change_public_key") {
		return command_change_public_key(msg, user);

	} else {
		answer = "unrecognized command";
	}
}

void c_market_server::command_status (const c_message &msg, const c_market_user &user) {
	answer = "not implemented yet\n";
}

void c_market_server::command_delete_account (const c_message &msg, const c_market_user &user) {
	users.remove_user(msg.sender_nickname);
	answer = "account deleted";
}

void c_market_server::command_register (const c_message &msg) {
	if (msg.sender_nickname.empty()) {
		answer = "empty nickname, aborting";
		return;
	}

	if (users.check_if_exists(msg.sender_nickname)) {
		answer = "user already registered, aborting";
		return;
	}

	c_market_user user;
	std::copy(msg.Crypto_data, msg.Crypto_data + 32, user.Public_key);
	users.add_user(msg.sender_nickname, user);

	answer = msg.sender_nickname + " registered";
}

void c_market_server::command_buy (const c_message &msg, const c_market_user &user) {
	t_price price = atoi(msg.Line[0].c_str());
	t_amount amount = atoi(msg.Line[1].c_str());

	auto result = market.buy(msg.sender_nickname, price, amount);
	answer = "You bought " + to_string(result.executed) + " tokens for total price " + to_string(result.total_price) +
					 " money.\n";

	if (amount - result.executed > 0)
		answer += to_string(amount - result.executed) + " offers is now waiting for execute\n";
}

void c_market_server::command_sell (const c_message &msg, const c_market_user &user) {
	t_price price = atoi(msg.Line[0].c_str());
	t_amount amount = atoi(msg.Line[1].c_str());

	auto result = market.sell(msg.sender_nickname, price, amount);
	answer =
		"You sold " + to_string(result.executed) + " tokens obtaining " + to_string(result.total_price) + " money.\n";

	if (amount - result.executed > 0)
		answer += to_string(amount - result.executed) + " offers is now waiting for execute\n";
}

void c_market_server::command_best_sell_offer () {
	answer = "Best sell offer is now " + to_string(market.get_best_sell_offer()) + "\n";
}

void c_market_server::command_best_buy_offer () {
	answer = "Best buy offer is now " + to_string(market.get_best_buy_offer()) + "\n";
}

void c_market_server::command_help () {
	answer = "ed<ed25591_signature>:register:<username>\n";
	answer += "ed<ed25591_signature>:sell <price> <amount>:<username>\n";
	answer += "ed<ed25591_signature>:buy <price> <amount>:<username>\n";
	answer += "best_sell_offer\n";
	answer += "best_buy_offer\n";
	answer += "ed<ed25591_signature>:status:<username>\n";
	answer += "ed<ed25591_signature>:delete_account:<username>\n";
	answer += "ed<ed25591_signature>:change_public_key <new_public_key>:<username>\n";
	answer += "help\n";
	answer += "<ed25519_signature> is always signature of everything behind the first colon";
}

bool c_market_server::verify_signature (const string &org_msg, const c_message &msg, const c_market_user &user) {
	if (msg.command == "best_buy_offer" || msg.command == "best_sell_offer" || msg.command == "help")
		return true;

	cout << "VERIFICATION:\n     Public Key: ";
	for (auto &v : user.Public_key)
		cout << (int)v << ' ';

	cout << "\n     Signature: ";
	for (size_t i = 0; i < 32; ++i)
		cout << (int)msg.Crypto_data[i] << ' ';

	cout << "\n     Original msg: ";
	cout << org_msg;
	cout << '\n' << "     Nickname: " << msg.sender_nickname << '\n';

	return crypto_api.verify_sign(org_msg, msg.Crypto_data, user.Public_key);
}

void c_market_server::command_change_public_key (const c_message &msg, const c_market_user &user) {
	command_delete_account(msg, user);
}

void c_market_server::start_demo () {
	while (true) {
		if (demo_request.empty())
			continue;

		cerr << "RECIEVED MESSAGE FROM LOCAL DEMO\n";
		cout << "message:     ";

		size_t length = demo_request.size();

		cout << demo_request << '\n';
		cout << "     length of message: " << length << '\n';

		answer.clear();
		init_execute_command(demo_request.c_str(), length, boost::asio::ip::address());
		if (answer.empty())
			answer = "some error occured";

		cerr << "answer is: " << answer << '\n';

		demo_request.clear();
	}
}
