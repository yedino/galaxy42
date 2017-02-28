
#pragma once

#include "libs0.hpp"

#include "transport/base/transp_base_obj.hpp" // parent
#include "transport/simulation/transp_simul_addr.hpp" // own addr
#include "transport/simulation/world.hpp"

class c_transport_simul_obj : public c_transport_base_obj {
	public:
		c_transport_simul_obj(shared_ptr<c_world> world);
		virtual ~c_transport_simul_obj();

		virtual void send_data(boost::any dest, const char *data, size_t size_of_data);

	protected:
		c_transport_simul_addr m_addr; ///< my own address
		shared_ptr<c_world> m_world;
};



