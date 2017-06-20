
#include <libs0.hpp>
#include <cable/kind.hpp>

std::string to_string(const t_cable_kind & obj) {
	switch (obj) {
		case (t_cable_kind::kind_simul):  return "simul"s;  break;
		case (t_cable_kind::kind_shm):  return "shm"s;  break;
		case (t_cable_kind::kind_udp):  return "udp"s;  break;
		default: break;
	}
	_throw_error_runtime("Unknown cable_kind object (in print)");
}

std::ostream & operator<<(std::ostream & ostr, const t_cable_kind & obj) {
	ostr << to_string(obj);
	return ostr;
}


