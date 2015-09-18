#include "c_users_database.hpp"

void c_users_database::add_user(const string &nickname, const c_market_user &user) {
	users.insert( {nickname, user} );
}

void c_users_database::remove_user (const string &nickname) {
	users.erase(nickname);
}

const c_market_user &c_users_database::get_user_by_nickname (const string &nickname) const {
	return users.at(nickname);
}

bool c_users_database::check_if_exists (const string &nickname) const {
	return (users.find(nickname) != users.end());
}
