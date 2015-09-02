#include "c_ring_shuffle.hpp"
#include "c_simulation.hpp"
#include "c_logger.hpp"

void c_simulation::print_connections (std::ostream &stream) {
	for (auto &v : users) {
		stream << "-------------- user " << v.current_user.name << ":\n"
		<< "---- prev: " << v.prev_user->name << '\n'
		<< "---- next: " << v.next_user->name << '\n';

		for (auto &u : v.ring)
			stream << "-> " << u.name << '\n';
	}
}

void c_simulation::start_simluation () {
	for (size_t i = 0; i < users.size(); ++i) {
		if (users.at(i).current_user.name != users.at(i).center_user->name)
			logger.send(users.at(i).current_user, *users.at(i).center_user, {users.at(i).current_user.pub_key});
	}

	users.at(0).center_user->inbox.push_back(users.at(0).center_user->pub_key);

	for (size_t i = 0; i < users.size(); ++i) {
		cout << (users.at(i).current_user.inbox.empty());
	}

	for (size_t i = 0; i < 10999L; ++i) // TODO
		tick();
}

void c_simulation::print_users (std::ostream &stream) {
	for (auto &v : users)
		stream << v.current_user.name << '\n';
}

void c_simulation::add_user () {
	c_crypto_RSA<256> crypto;
	crypto.generate_key();
	c_user user;
	user.name = std::to_string(users.size());
	user.pub_key = crypto.get_public_key();
	c_ring_shuffle shuffler(user);
	users.push_back(shuffler);
}

void c_simulation::initialize_shufflers () {
	for (size_t i = 0; i < users.size(); ++i) {
		for (size_t li = 0; li < i; ++li) {
			users.at(i).add_user(users.at(li).current_user);
		}

		for (size_t ri = i + 1; ri < users.size(); ++ri) {
			users.at(i).add_user(users.at(ri).current_user);
		}
	}
}

void c_simulation::initialize_simulation () {
	initialize_shufflers();
	for (auto &v : users)
		v.initialize_shuffle();

	users.shrink_to_fit();
}

void c_simulation::tick () {
	for (size_t i = 0; i < users.size(); ++i) {
		if (users.at(i).current_user.inbox.empty())
			continue;

		cout << ":)";
		shuffle_msgs(users.at(i).current_user.inbox);
		logger.send(users.at(i).current_user, *users.at(i).next_user, users.at(i).current_user.inbox);
		users.at(i).current_user.inbox = {};
	}
}

void c_simulation::shuffle_msgs (vector<public_key<c_crypto_RSA<256>::long_type>> &msgs) {
	for (size_t i = 0; i < msgs.size(); ++i)
		std::swap(msgs.at(rand() % msgs.size()), msgs.at(rand() % msgs.size()));
}


/* TODO:
 * [DONE] wybrac pierwszego ziomka
 * ticka naprawic
 */