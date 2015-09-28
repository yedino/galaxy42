#ifndef SERVER_USERS_DATABASE_HPP
#define SERVER_USERS_DATABASE_HPP
#include <unordered_map>
#include "c_market_user.hpp"

using std::string;
using std::unordered_map;

class c_users_database {
private:
		unordered_map<string, c_market_user> users;

		static const size_t npos = 0;

public:
		void add_user (const string &, const c_market_user &);

		void remove_user (const string &);

		const c_market_user &get_user_by_nickname (const string &) const;

		bool check_if_exists (const string &) const;
};


#endif //SERVER_USERS_DATABASE_HPP
