
#pragma once

#include <cable/base/cable_base_addr.hpp>
#include <cable/base/cable_base_obj.hpp>
#include <cable/kind.hpp>
#include <libs0.hpp>

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
 */

class c_card_selector {
	public:
		c_card_selector(t_cable_kind kind, unique_ptr<c_cable_base_addr> && my_addr);

		c_card_selector(const c_card_selector & other);
		c_card_selector & operator=(const c_card_selector & other);

		bool operator==(const c_card_selector & other) const;
		bool operator!=(const c_card_selector & other) const;
		bool operator<(const c_card_selector & other) const;

		void print(ostream & out) const;

		auto & get_kind() { return m_kind; }
		const auto & get_kind() const { return m_kind; }
		auto & get_my_addr() { return m_my_addr; }
		const auto & get_my_addr() const { return m_my_addr; }

		friend class m_cable_cards; ///< to read addr etc without getters TODO

	private:
		t_cable_kind m_kind;
		unique_ptr<c_cable_base_addr> m_my_addr; ///< polymorphic to have any address type
};

ostream & operator<<(ostream & out, const c_card_selector & obj);

