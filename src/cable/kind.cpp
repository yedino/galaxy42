
#pragma once

#include <libs0.hpp>

typedef enum {
	e_cable_kind_simul=1,
	e_cable_kind_shm=2,
	e_cable_kind_udp=3,
} t_cable_kind;

std::ostream & operator<<(std::ostream & ostr, t_cable_kind obj) {
	switch (obj) {
		case (e_cable_kind_simul):
			ostr << "simul";
		break;
		case (e_cable_kind_shm):
			ostr << "shm";
		break;
		case (e_cable_kind_udp):
			ostr << "udp";
		break;
		default: _throw_error_runtime("Unknown cable_kind object (in print)");
	}
	return ostr;
}


