#ifndef CABLE_SHM_OBJ_HPP
#define CABLE_SHM_OBJ_HPP

#include "../base/cable_base_obj.hpp"
#include "cable_shm_addr.hpp"

class cable_shm_obj : c_cable_base_obj {
	public:
		cable_shm_obj();
		void send_to(const c_cable_base_addr & dest, const unsigned char *data, size_t size) override; ///< block function
};

#endif // CABLE_SHM_OBJ_HPP
