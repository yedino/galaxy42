#include "c_user.hpp"

bool operator== (const c_user &lhs, const c_user &rhs) {
	return (lhs.pub_key == rhs.pub_key && lhs.name == rhs.name);
}

bool operator!= (const c_user &lhs, const c_user &rhs) { return !(lhs == rhs); }