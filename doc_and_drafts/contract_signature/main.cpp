#include <iostream>
#include "c_crypto_ed25519.hpp"
#include "c_user.hpp"
#include "c_contract.hpp"
#include <list>
#include <vector>
#include "c_random_generator.hpp"
#include <algorithm>
#include <iomanip>

#define DBG(x) { cout << left << setw(50) << #x << boolalpha << (x) << endl; }

using namespace std;

string generate_nickname () {
	static size_t number = 0;
	return "user_" + to_string(number++);
}

template <typename T = long long>
T get_random () {
	static c_random_generator<T> generator;
	return generator.get_random(sizeof(T));
}

void simple_test () {
	c_user user_1(generate_nickname()), user_2(generate_nickname());

	c_contract contract(user_1, user_2, 155);
	contract.create_msg();

	cout << contract.message;
	contract.create_signature();
	cout << '\n' << contract.verify_signature();
}

bool serialization_test () {
	c_user user_1(generate_nickname()), user_2(generate_nickname());
	c_contract org(user_1, user_2, 155);
	org.create_msg();
	org.create_signature();
	c_contract dup = c_contract::from_string(org.to_string());
	cout << org.to_string() << '\n' << dup.to_string();
	return true;
}

vector<c_user> generate_random_route (size_t hops = 100) {
	vector<c_user> route;

	for (size_t i = 0; i < hops; ++i)
		route.emplace_back(generate_nickname());

	for (size_t i = route.size() - 1; i > 0; --i)
		swap(route.at(i), route.at(get_random<size_t>() % (route.size() - 1)));

	return route;
}

vector<c_contract> generate_token_contracts (vector<c_user> &route, size_t data_size, size_t amount) {
	vector<c_contract> contracts;
	for (size_t i = 1; i < route.size(); ++i) {
		contracts.emplace_back(route.at(i - 1), route.at(i), data_size);
		route.at(i - 1).send_token(route.at(i), amount);
	}

	for (c_contract &c : contracts) {
		c.create_msg();
		c.create_signature();
	}

	return contracts;
}

bool verify_contracts (const vector<c_contract> &contracts) {
	for (const c_contract &c : contracts) {
		if (!c.verify_signature())
			return false;
	}
	return true;
}

void send_contracts (const vector<c_contract> &contracts) {

}

void simulation () {
	auto route = generate_random_route(100);
	cout << "ROUTE:\n";
	cout << " start node: " << route.begin()->nickname << "\n             ";
	for (c_user &c : route)
		cout << c.nickname << "  ";

	cout << "\n end node  : " << prev(route.end())->nickname << "\n\n";

	route.at(0).emit_tokens(10);

	auto contracts = generate_token_contracts(route, 155, 1);
	cout << (verify_contracts(contracts) ? "verifying OK\n" : "verifying gone WRONG\n");
}


int main () {
	simulation();
	return 0;
}