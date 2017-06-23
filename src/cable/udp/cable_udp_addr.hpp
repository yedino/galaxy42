#pragma once
#ifndef cable_UDP_ADDR_HPP
#define cable_UDP_ADDR_HPP

#include "../base/cable_base_addr.hpp"
#include <boost/asio.hpp>

class c_cable_udp_addr final : public c_cable_base_addr {
	public:
		using t_addr = boost::asio::ip::udp::endpoint; ///< actuall raw address. Instace of this will be in m_addrdata

		c_cable_udp_addr() = default;
		c_cable_udp_addr(const t_addr &endpoint);
		c_cable_udp_addr(const std::string &ip_string); ///< Converts string like "100.200.50.50:32000"

		virtual unique_ptr<c_cable_base_addr> clone() const override; ///< polymorphic clone

		void print(std::ostream & ostr) const override;

		inline t_addr & get_addr();
		inline const t_addr & get_addr() const;

	protected:
		t_addr m_addr;
		virtual signed char compare_same_class(const c_cable_base_addr & other) const override;
};

// ===========================================================================================================
// implementation

inline c_cable_udp_addr::t_addr & c_cable_udp_addr::get_addr() { return m_addr; }
inline const c_cable_udp_addr::t_addr & c_cable_udp_addr::get_addr() const { return m_addr; }

#endif // cable_UDP_ADDR_HPP
