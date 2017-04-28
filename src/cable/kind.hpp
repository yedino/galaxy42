
#pragma once

#include <iostream>

typedef enum {
	e_cable_kind_simul=1,
	e_cable_kind_shm=2,
	e_cable_kind_udp=3,
} t_cable_kind;

std::ostream & operator<<(std::ostream & ostr, t_cable_kind obj);

