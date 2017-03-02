
#pragma once

#include "libs0.hpp"

#include "transport/base/transp_base_obj.hpp" // parent
#include "transport/simulation/transp_simul_addr.hpp" // own addr
#include "transport/simulation/world.hpp"

class c_transport_simul_obj : public c_transport_base_obj {
	public:
		c_transport_simul_obj(shared_ptr<c_world> world);
		virtual ~c_transport_simul_obj();

		void send_to(const c_transport_base_addr & dest, const unsigned char *data, size_t size) override;
		size_t receive_from(c_transport_base_addr & source, unsigned char * const data, size_t size) override;
		void listen_on(c_transport_base_addr & local_address) override;

	protected:
		c_transport_simul_addr m_addr; ///< my own address
		shared_ptr<c_world> m_world; ///< world in which me (and my peers) exist
};



