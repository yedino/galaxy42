#include "c_logger.hpp"

c_logger::c_logger (ostream &stream) : output(stream) { }

void c_logger::send (c_user &a, c_user &b, const vector<public_key<c_crypto_RSA<256>::long_type>> &msgs) {
	cout << a.name << " sends msg to " << b.name << '\n';

	for (auto &v : msgs)
		b.inbox.push_back(v);
}
