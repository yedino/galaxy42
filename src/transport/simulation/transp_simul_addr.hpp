
#pragma once

#include "libs0.hpp"
#include "transport/base/transp_base_addr.hpp" // base

class c_transport_simul_addr : public c_transport_base_addr {
	public:
		c_transport_simul_addr(int uuid);

		virtual void print(ostream & ostr) const ;

		int m_uuid;
};


