#include "trivialserialize_test_object.hpp"

namespace trivialserialize {

std::ostream& operator<<(std::ostream& out, const c_tank & t) {
	out << "[" << t.ammo << ' ' << t.speed << ' ' << t.name << "]";
	return out;
}
bool operator==(const c_tank & aaa, const c_tank & bbb) {
	return (aaa.ammo == bbb.ammo) && (aaa.speed == bbb.speed) && (aaa.name == bbb.name);
}
bool operator<(const c_tank & aaa, const c_tank & bbb) {
	if (aaa.ammo < bbb.ammo) return 1;
	if (aaa.ammo > bbb.ammo) return 0;
	if (aaa.speed < bbb.speed) return 1;
	if (aaa.speed > bbb.speed) return 0;
	if (aaa.name < bbb.name) return 1;
	if (aaa.name > bbb.name) return 0;
	return 0; // the same
}
}
