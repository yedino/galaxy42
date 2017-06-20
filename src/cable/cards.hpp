#pragma once

#include <cable/base/cable_base_addr.hpp>
#include <cable/base/cable_base_obj.hpp>
#include <cable/asio_ioservice_manager.hpp>
#include <cable/kind.hpp>
#include <cable/selector.hpp>
#include <libs0.hpp>

/** Group of cable-cards (virtual transport network "card"),
 * group of objects like the object used to send/receive UDP transport data, other one for TCP etc
 * 1 Card is an object, that can be used to send/receive from anyone (to any destination address) and that is
 * the only difference.
 * So e.g. when you have other SOURCE address (local address), then it will be other card.
 * Such cards can be spawned on demand as need comes.
 */
class c_cable_cards final {
	public:
		~c_cable_cards()=default;

		/// @return Return (spawn if needed) the card for given seletor (e.g. cable type + my ip:port, e.g. UDP,any,9042)
		/// @warning Returned reference can be invalided on next call, use immediatelly!
		c_cable_base_obj & get_card(const c_card_selector & selector);

		void stop_threadsafe(); ///< tries to stop all io_services, and all cards. [thread_safe]

	private:
		unique_ptr<c_cable_base_obj> create_card(const c_card_selector & selector); ///< factory for cards

		map< c_card_selector , unique_ptr<c_cable_base_obj> > m_cards;

		shared_ptr<c_asioservice_manager_base> m_asioservice_manager; ///< needed to create cables based on asio service

		shared_ptr<c_asioservice_manager_base> & get_asioservice(); ///< creates if needed and returns an asioservice manager
};


