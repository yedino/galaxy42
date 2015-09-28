#include "c_contract.hpp"

void c_contract::create_msg () {
	message = user_a.nickname + "|" + std::to_string(data_size) + "kB|" + user_b.nickname;
}

bool c_contract::verify_signature () const {
	return user_b.crypto_api.verify_sign(message, Signature, user_a.get_public_key());
}

void c_contract::create_signature () {
	user_a.crypto_api.sign(message, Signature);
}

c_contract::c_contract (const c_user &a,
	const c_user &b,
	size_t data_length) : user_a(a), user_b(b), data_size(data_length) { }

string c_contract::to_string () const {
	string result;
	result += message;

	for (short i = 0; i < 64; ++i) {
		result += "|" + std::to_string((int)Signature[i]);
	}
	return result;
}

c_contract c_contract::from_string (const string &value) {
	string nick_a;
	size_t x = 0, i;
	for (i = 0; i < value.size(); ++i) {
		if (value.at(i) == '|') {
			break;
		} else {
			nick_a += value.at(i);
		}
	}

	size_t data_size = 0;
	for (++i; i < value.size(); ++i) {
		if (value.at(i) == 'k') {
			break;
		} else {
			data_size *= 10;
			data_size += (int)value.at(i) - '0';
		}
	}

	string nick_b;
	for (i += 3; i < value.size(); ++i) {
		if (value.at(i) == '|') {
			break;
		} else {
			nick_b += value.at(i);
		}
	}

	c_user user_a(nick_a), user_b(nick_b);
	c_contract result(user_a, user_b, data_size);
	short s_i = 0, data = 0;
	for (++i; i < value.size(); ++i) {
		if (value.at(i) == '|') {
			result.Signature[s_i] = (unsigned char)data;
			data = 0;
			++s_i;
		} else {
			data *= 10;
			data += (int)value.at(i) - '0';
		}
	}
	return result;
}

void swap (c_contract &lhs, c_contract &rhs) {
	using std::swap;
//	swap(rhs.user_a, lhs.user_a);
//	swap(rhs.user_b, lhs.user_b);
	swap(rhs.data_size, lhs.data_size);
	swap(rhs.Signature, lhs.Signature);
}

c_contract::c_contract (const c_contract &other) : user_a(other.user_a), user_b(other.user_b), data_size(other.data_size) {
	for (short i = 0; i < 64; ++i)
		Signature[i] = other.Signature[i];
}

c_contract::c_contract (c_contract &&contract) : c_contract() { swap(*this, contract); }

c_contract::c_contract () : user_a(c_user(" ")), user_b(c_user(" ")) { }

c_contract &c_contract::operator= (c_contract rhs) {
	swap(*this, rhs);
	return *this;
}
