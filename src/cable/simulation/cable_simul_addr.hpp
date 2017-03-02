
#pragma once

#include "libs0.hpp"
#include "cable/base/cable_base_addr.hpp" // base

class c_cable_simul_addr : public c_cable_base_addr {
	public:
		typedef int t_addr; ///< actuall type of my raw address. Instace of this will be in m_addrdata (::any)

		c_cable_simul_addr(t_addr uuid);

		virtual void print(ostream & ostr) const ;
};

