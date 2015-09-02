#include "c_market_server.hpp"

void c_market_server::work (boost::asio::io_service &io_service) {
	udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
	string answer;
	for (; ;) {
		char data[max_length];
		udp::endpoint sender_endpoint;
		size_t length = sock.receive_from(boost::asio::buffer(data, max_length), sender_endpoint);
		answer = execute_command(data, length);
		cerr << "command executed\n";
		sock.send_to(boost::asio::buffer(answer.c_str(), answer.size()), sender_endpoint); // works?
	}
}

void c_market_server::start () {
	try {
		boost::asio::io_service io_service;
		work(io_service);
	} catch (std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
	}
}

string c_market_server::execute_command (char *data, size_t length) {
	string command = get_command(data, length, 0);
	if (command == "best_buy_offer") {
		return command_best_buy_offer();

	} else if (command == "best_sell_offer") {
		return command_best_sell_offer();

	} else if (command == "sell") {
		return command_sell(data, length);

	} else if (command == "buy") {
		return command_buy(data, length);

	} else if (command == "register") {
		return command_register(data, length);

	} else if (command == "help") {
		return command_help();

	} else if (command == "login") {
		return command_login(data, length);

	} else if (command == "logout") {
		return command_logout(data, length);

	} else if (command == "delete_account") {
		return command_delete_account(data, length);

	} else if (command == "status") {
		return command_status(data, length);

	} else {
		return "unrecognized command\n";
	}
}

string c_market_server::get_command (char *data, size_t length, size_t command_id) {
	string command;
	size_t i;
	for (i = 0; i < length && data[i] != ':'; ++i);
	for (size_t c = 0; c < command_id; ++c) {
		++i;
		for (; i < length && data[i] != ' '; ++i);
	}
	for (++i; i < length && data[i] != ' '; ++i)
		command += data[i];
	return command;
}

c_market_server::c_market_server (unsigned short port) : port(port), market(c_market()) {
	//	crypto.generate_key();
}

c_crypto<2048>::long_type c_market_server::get_sign (char *&data, size_t &length) {
	string s_sign;
	size_t i;
	for (i = 0; i < length && data[i] != ':'; ++i)
		s_sign += data[i];

	++i;
	data += i;
	length -= i;
	return c_crypto<2048>::long_type(s_sign);
}

public_key<c_crypto<2048>::long_type> c_market_server::get_public_key (char *data, size_t length) {
	string s_e, s_n;
	size_t i;
	for (i = 0; i < length && data[i] != '|'; ++i)
		s_e += data[i];

	for (++i; i < length && data[i] != ':'; ++i)
		s_n += data[i];

	return {c_crypto<2048>::long_type(s_e), c_crypto<2048>::long_type(s_n)};
}



string c_market_server::command_status (char *data, size_t length) {
	return "not implemented yet\n";
}

string c_market_server::command_delete_account (char *data, size_t length) {
	string ret;
	if (verify_user(data, length))
		ret = "verified command\n";
	else
		return "verification error";

	string login = get_command(data, length, 1);
	string password = get_command(data, length, 2);
	if (users.find(login) == users.end())
		return ret + "no such user " + login + "\n";

	if (!crypto.verify_hash(password, users.find(login)->second))
		return ret + "invalid password\n";
	else {
		if (logged_on_users.find(login) != logged_on_users.end())
			logged_on_users.erase(login);

		users.erase(login);
		return ret + "done\n";
	}
}

string c_market_server::command_logout (char *data, size_t length) {
	string ret;
	if (verify_user(data, length))
		ret = "verified command\n";
	else
		return "verification error";

	string login = get_command(data, length, 1);
	string password = get_command(data, length, 2);
	if (logged_on_users.find(login) == logged_on_users.end())
		return ret + "not logged on\n";

	if (!crypto.verify_hash(password, users.find(login)->second))
		return ret + "invalid password\n";
	else {
		logged_on_users.erase(login);
		return ret + "done\n";
	}
}

string c_market_server::command_login (char *data, size_t length) {
	string login = get_command(data, length, 1);
	string password = get_command(data, length, 2);

	if (logged_on_users.find(login) != logged_on_users.end())
		return "already logged on\n";

	if (users.find(login) == users.end())
		return "user not registered\n";

	if (!crypto.verify_hash(password, users.find(login)->second))
		return "invalid password\n";
	else {
		auto pub_key = get_public_key(data, length);
		logged_on_users.insert({login, pub_key});
		return "done\n";
	}
}

string c_market_server::command_help () {
	string ret;
	ret = "avaliable commands:\n";
	ret += "register <login> <password>\n";
	ret += "login <login> <password>\n";
	ret += "sell <login> <price> <amount>\n";
	ret += "buy <login> <price> <amount>\n";
	ret += "get_best_buy_offer\n";
	ret += "get_best_sell_offer\n";
	ret += "status <login> <password>\n";
	ret += "logout <login> <password>\n";
	ret += "delete_account <login> <password>\n";
	return ret;
}

string c_market_server::command_register (char *data, size_t length) {
	string login = get_command(data, length, 1);
	string password = get_command(data, length, 2);

	if (users.find(login) != users.end())
		return "user already exists\n";
	else {
		users.insert({login, crypto.generate_hash(password)});
		return "done";
	}
}

string c_market_server::command_buy (char *data, size_t length) {
	string ret;
	string trader = get_command(data, length, 1);
	if (logged_on_users.find(trader) == logged_on_users.end())
		return "not logged on\n";

	if (verify_user(data, length))
		ret = "verified command\n";
	else
		return "verification error";

	t_price price = atoi(get_command(data, length, 2).c_str());
	t_amount amount = atoi(get_command(data, length, 3).c_str());

	auto result = market.buy(trader, price, amount);
	ret += "You bought " + to_string(result.executed) + " tokens for total price " + to_string(result.total_price) +
	      " money.\n";

	if (amount - result.executed > 0)
		ret += to_string(amount - result.executed) + " offers is now waiting for execute\n";

	return ret;
}

string c_market_server::command_sell (char *data, size_t length) {
	string ret;
	string trader = get_command(data, length, 1);
	if (logged_on_users.find(trader) == logged_on_users.end())
		return "not logged on\n";

	if (verify_user(data, length))
		ret = "verified command\n";
	else
		return "verification error";

	t_price price = atoi(get_command(data, length, 2).c_str());
	t_amount amount = atoi(get_command(data, length, 3).c_str());

	auto result = market.sell(trader, price, amount);
	ret += "You sold " + to_string(result.executed) + " tokens obtaining " + to_string(result.total_price) + " money.\n";
	if (amount - result.executed > 0)
		ret += to_string(amount - result.executed) + " offers is now waiting for execute\n";

	return ret;
}

string c_market_server::command_best_sell_offer () {
	return "Best sell offer is now " + to_string(market.get_best_sell_offer()) + "\n";
}

string c_market_server::command_best_buy_offer () {
	return "Best buy offer is now " + to_string(market.get_best_buy_offer()) + "\n";
}

bool c_market_server::verify_user (char *&data, size_t &length) {
	auto signature = get_sign(data, length);
	string trader = get_command(data, length, 1);
	auto pub_key = logged_on_users.at(trader);
	return crypto.verify_sign(string(data, length), signature, pub_key.e, pub_key.n);
}
