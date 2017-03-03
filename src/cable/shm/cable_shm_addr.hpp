#ifndef CABLE_SHM_ADDR_HPP
#define CABLE_SHM_ADDR_HPP

#include "../base/cable_base_addr.hpp"

class cable_shm_addr final : c_cable_base_addr {
	public:
		typedef std::string t_addr; ///< actuall raw address. Instace of this will be in m_addrdata
		cable_shm_addr() = default;
		cable_shm_addr(const std::string &address);
};

#endif // CABLE_SHM_ADDR_HPP
