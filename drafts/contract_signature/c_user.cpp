#include "c_user.hpp"

c_user::c_user (const string &nick) : nickname(nick) {
	if (nickname.find('|') != string::npos) {
		throw runtime_error("nickname is not allowed to contain '|' character");
	}

	crypto_api.generate_key();
}

const unsigned char *const c_user::get_public_key () const {
	return crypto_api.get_public_key();
}

c_user::c_user (string &&nick) {
	nickname.swap(nick);
	if (nickname.find('|') != string::npos) {
		throw runtime_error("nickname is not allowed to contain '|' character");
	}

	crypto_api.generate_key();
}
