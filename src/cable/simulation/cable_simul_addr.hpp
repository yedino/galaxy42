
#pragma once

#include "libs0.hpp"
#include "cable/base/cable_base_addr.hpp" // base

class c_cable_simul_addr : public c_cable_base_addr {
	public:
		using t_addr = int; ///< actuall type of my raw address. Instace of this will be in m_addrdata (::any)

		c_cable_simul_addr(t_addr uuid);
		virtual unique_ptr<c_cable_base_addr> clone() const override; ///< polymorphic clone

		virtual void print(ostream & ostr) const override;

		inline t_addr get_addr() const;

	protected:
		t_addr m_addr;
		virtual signed char compare_same_class(const c_cable_base_addr & other) const override;
};

// ===========================================================================================================
// implementation

inline c_cable_simul_addr::t_addr c_cable_simul_addr::get_addr() const { return m_addr; }

