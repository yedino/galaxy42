
#pragma once

#include "libs0.hpp"

#include "cable/base/cable_base_obj.hpp" // parent
#include "cable/simulation/cable_simul_addr.hpp" // own addr
#include "cable/simulation/world.hpp"

class c_cable_simul_obj : public c_cable_base_obj {
	public:
		c_cable_simul_obj(shared_ptr<c_world> world);
		virtual ~c_cable_simul_obj();

		void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) override;
		void async_send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size, write_handler handler) override;
		size_t receive_from(c_cable_base_addr & source, unsigned char * const data, size_t size) override;
		void async_receive_from(unsigned char * const data, size_t size, read_handler handler) override;
		void listen_on(c_cable_base_addr & local_address) override;

	protected:
		c_cable_simul_addr m_addr; ///< my own address
		shared_ptr<c_world> m_world; ///< world in which me (and my peers) exist
};



