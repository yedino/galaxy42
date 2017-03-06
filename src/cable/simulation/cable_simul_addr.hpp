
#pragma once

#include "libs0.hpp"
#include "cable/base/cable_base_addr.hpp" // base

class c_cable_simul_addr : public c_cable_base_addr {
	public:
		typedef int t_addr; ///< actuall type of my raw address. Instace of this will be in m_addrdata (::any)

		c_cable_simul_addr(t_addr uuid);

		virtual void print(ostream & ostr) const ;

		std::string cable_type_name() const override; ///< return name of type of this cable, e.g.: "tcp" "udp" "ETH" "shm"

		/// is my address the same (cable type, and address) to another.
		virtual bool is_same(const c_cable_base_addr &other) const;

		/// return -1 if I am smaller, 0 if same, +1 if bigger, then the other address. Compares also across different cable-types
		virtual int compare(const c_cable_base_addr &other) const;
};

