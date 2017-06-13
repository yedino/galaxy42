#ifndef CABLE_SHM_ADDR_HPP
#define CABLE_SHM_ADDR_HPP

#include "../base/cable_base_addr.hpp"

class cable_shm_addr final : public c_cable_base_addr {
	public:
		using t_addr = std::string; ///< actuall raw address data-type

		cable_shm_addr();
		cable_shm_addr(const std::string &address);

		virtual void print(std::ostream & ostr) const override; ///< displays human readable form of this address

		virtual unique_ptr<c_cable_base_addr> clone() const override; ///< polymorphic clone

	private:
		t_addr m_addr; ///< actuall address

		virtual signed char compare_same_class(const c_cable_base_addr & other) const override;
};

#endif // CABLE_SHM_ADDR_HPP
