#ifndef CABLE_SHM_ADDR_HPP
#define CABLE_SHM_ADDR_HPP

#include "../base/cable_base_addr.hpp"

class cable_shm_addr final : c_cable_base_addr {
	public:
		typedef std::string t_addr; ///< actuall raw address. Instace of this will be in m_addrdata
		cable_shm_addr() = default;
		cable_shm_addr(const std::string &address);
		/// is my address the same (cable type, and address) to another.
		bool is_same(const c_cable_base_addr &other) const override;

		/// return -1 if I am smaller, 0 if same, +1 if bigger, then the other address. Compares also across different cable-types
		int compare(const c_cable_base_addr &other) const override;
};

#endif // CABLE_SHM_ADDR_HPP
