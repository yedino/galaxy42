
#include <cable/kind.hpp>
#include <tnetdbg.hpp>

std::string to_string(const t_cable_kind & obj) {
	using namespace std::string_literals;
	switch (obj) {
		case (t_cable_kind::kind_simul):  return "simul"s;  break;
		case (t_cable_kind::kind_shm):  return "shm"s;  break;
		case (t_cable_kind::kind_udp):  return "udp"s;  break;
		default: break;
	}
	pfp_throw_error_runtime("Unknown cable_kind object (in print)");
}

std::ostream & operator<<(std::ostream & ostr, const t_cable_kind & obj) {
	ostr << to_string(obj);
	return ostr;
}


