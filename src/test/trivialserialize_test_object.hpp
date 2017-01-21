#ifndef TRIVIALSERIALIZE_TEST_OBJECT_HPP
#define TRIVIALSERIALIZE_TEST_OBJECT_HPP

#include <ostream>
#include <string>
#include "../trivialserialize.hpp"

namespace trivialserialize {

struct c_tank {
	int ammo;
	int speed;
	std::string name;
};

std::ostream& operator<<(std::ostream& out, const c_tank & t);
bool operator==(const c_tank & aaa, const c_tank & bbb);
bool operator<(const c_tank & aaa, const c_tank & bbb);

template <> inline void obj_serialize(const c_tank & data , trivialserialize::generator & gen) {
	gen.push_integer_uvarint(data.ammo);
	gen.push_integer_uvarint(data.speed);
	gen.push_varstring(data.name);
}

template <> inline c_tank obj_deserialize<c_tank>(trivialserialize::parser & parser) {
	c_tank ret;
	ret.ammo = parser.pop_integer_uvarint();
	ret.speed = parser.pop_integer_uvarint();
	ret.name = parser.pop_varstring();
	return ret;
}

} // namespace

#endif // TRIVIALSERIALIZE_TEST_OBJECT_HPP
