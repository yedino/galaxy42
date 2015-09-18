#ifndef SERVER_C_MARKET_USER_H
#define SERVER_C_MARKET_USER_H
#include <string>

using std::string;

struct c_market_user {
		unsigned char Public_key[32];

		c_market_user () { }

		bool operator== (const c_market_user &rhs) const {
			for (size_t i = 0; i < 32; ++i) {
				if (Public_key[i] != rhs.Public_key[i])
					return false;
			}
			return true;
		}

		bool operator!= (const c_market_user &rhs) const { return !(*this == rhs); }
};
#endif //SERVER_C_MARKET_USER_H
