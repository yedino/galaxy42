#include "c_ring_shuffle.hpp"

c_ring_shuffle::c_ring_shuffle (const c_user &user) : current_user(user) { }

void c_ring_shuffle::synchronize_rings () {
	ring.sort([] (const c_user &a, const c_user &b) {
			return a.pub_key.n < b.pub_key.n;
	});

	// TODO there are better ideas to do that
}

void c_ring_shuffle::set_my_place_in_group () { // TODO optimization
	auto it = ring.begin();
	while (*it != current_user) {
		if (it == ring.end())
			it = ring.begin();
		else
			++it;
	}

	prev_user = this->prev(it);
	next_user = this->next(it);
}

void c_ring_shuffle::initialize_shuffle () {
	ring.push_back(current_user);
	synchronize_rings();
	set_center_user();
	set_my_place_in_group();
	generate_new_identity();
}

void c_ring_shuffle::shift () {

}

void c_ring_shuffle::start_shuffle () {
	if (*center_user == current_user) {
		for (auto &user : ring) {
			current_user.inbox.push_back(network_driver.recieve(current_user, user)); // TODO implement func recieve()
		}
		shift();
	}

	else {
		network_driver.send(current_user, *center_user, {encrypt_msg(current_user.pub_key)}); // TODO public_key structure
	}
}

void c_ring_shuffle::generate_new_identity () { new_identity.generate_key(); }

void c_ring_shuffle::add_user (const c_user &user) { ring.push_back(user); }

void c_ring_shuffle::add_user (c_user &&user) { ring.push_back(user); }

void c_ring_shuffle::set_center_user () {
	center_user = ring.begin();
	// TODO
}

c_crypto_RSA<256>::long_type c_ring_shuffle::encrypt_msg (const string &msg) {
	auto encrypted = encryption_driver.encrypt(msg, center_user->pub_key);
	auto it = this->next(center_user);
	auto backup = it;
	do {
		encryption_driver.encrypt(encrypted, it->pub_key); // TODO
	} while (it != backup);

	return encrypted;
}

list<c_user>::iterator c_ring_shuffle::next (const list<c_user>::iterator &it) {
	if (std::next(it) == ring.end())
		return ring.begin();

	return std::next(it);
}

list<c_user>::iterator c_ring_shuffle::prev (const list<c_user>::iterator &it) {
	if (std::prev(it) == ring.end())
		return std::prev(ring.end());

	return std::prev(it);
}
