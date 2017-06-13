
#pragma once

#include <cable/base/cable_base_addr.hpp>
#include <cable/base/cable_base_obj.hpp>
#include <cable/kind.hpp>
#include <libs0.hpp>

/**
 * @brief The c_card_selector_base class needed for create mock
 */
class c_card_selector_base {
	public:
		virtual ~c_card_selector_base() = default;
		virtual void print(ostream & out) const = 0;
		virtual t_cable_kind get_kind() const = 0;
		virtual c_cable_base_addr & get_my_addr() = 0;
		virtual const c_cable_base_addr & get_my_addr() const = 0;
};

/**
 * This are selectors that choose which card to use to send with or to listen on.
 * For example "from udp 192.168.0.5 port 9042" or "from udp, any IP, port 9042",
 *
 * Currently we have 1 card per each protocol + host ip:port
 * It is needed that way, e.g. for sending data FROM us, because we choose the my ip:port (source ip:port when I send)
 * by binding on the socket() so need separate socket() objects,
 * therefore we can no longer just use one card for sending all UDP packets (if we want to send from different my source ip:port).
 *
 * It is also needed when listening, so that we will listen on specifi one IP for example.
 *
 * This provides comparsion operators op< op== op!= that work on polymorphic address type,
 * and this class here is final, so it allows to be held as not-polymorphic key and still do polymorphic address comparsion.
 * e.g.:
 *   map< c_card_selector , ... >
 * works as well as
 *   map< unique_ptr< c_cable_base_addr > , ... , {comparator that dereferences keys and compares them} >
 *
 */

class c_card_selector final : public c_card_selector_base {
	public:
		c_card_selector()=default;
		c_card_selector(unique_ptr<c_cable_base_addr> && my_addr);

		c_card_selector(const c_card_selector & other);
		c_card_selector & operator=(const c_card_selector & other);

		/// @{ comparsion operator, on polymorphic address
		bool operator==(const c_card_selector & other) const;
		bool operator!=(const c_card_selector & other) const;
		bool operator<(const c_card_selector & other) const;
		/// @}

		void print(ostream & out) const override;

		inline t_cable_kind get_kind() const override;
		inline c_cable_base_addr & get_my_addr() override;
		inline const c_cable_base_addr & get_my_addr() const override;

	private:
		unique_ptr<c_cable_base_addr> m_my_addr; ///< polymorphic to have any address type
};

ostream & operator<<(ostream & out, const c_card_selector & obj);

t_cable_kind c_card_selector::get_kind() const { return UsePtr(m_my_addr).get_kind(); }
c_cable_base_addr & c_card_selector::get_my_addr() { return UsePtr(m_my_addr); }
const c_cable_base_addr & c_card_selector::get_my_addr() const { return UsePtr(m_my_addr); }
/// [optimize-maybe] could remove this UsePtr if other functions guarantee object is in consistent state and that is not-null

