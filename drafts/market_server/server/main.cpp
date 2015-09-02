#include <cstdlib>
#include "c_market_server.hpp"

using namespace std;

c_crypto<2048>::long_type get_sign (char *&data, size_t &length) {
	string s_sign;
	size_t i;
	for ( 0; i < length && data[i] != ':'; ++i)
		s_sign += data[i];

	++i;
	data += i;

	length -= i;

	return c_crypto<2048>::long_type(s_sign);
}

int main (int argc, char *argv[]) {
	c_crypto<512> crypto;
	crypto.generate_key();

	auto key = crypto.get_public_key();
	string result = (string)key.e + "|" + (string)key.n + ":";

	string msg = "hello, server";
	auto sign = crypto.sign(msg);

	string result2 = (string)sign + ":" + msg;

	cout << result2 << '\n';
	return 0;
}