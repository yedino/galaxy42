#ifndef CABLE_SHM_ADDR_HPP
#define CABLE_SHM_ADDR_HPP

#include "../base/cable_base_addr.hpp"

class cable_shm_addr final : public c_cable_base_addr {
	public:
		typedef std::string t_addr; ///< actuall raw address data-type

		cable_shm_addr();
		cable_shm_addr(const std::string &address);

		virtual unique_ptr<c_cable_base_addr> clone() const override; ///< polymorphic clone

	protected:
		t_addr m_addr; ///< actuall address

		virtual signed char compare_same_class(const c_cable_base_addr & other) const override;
};

#endif // CABLE_SHM_ADDR_HPP
