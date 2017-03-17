#pragma once

#include "base/cable_base_addr.hpp"
#include "base/cable_base_obj.hpp"
#include "asio_ioservice_manager.hpp"
#include "kind.hpp"
#include "../libs0.hpp"

/// Group of cable-cards (virtual transport network "card"),
/// group of objects like the object used to send/receive UDP transport data, other one for TCP etc
/// Such cards can be spawned on demand as need comes.
class c_cable_cards {
	public:
		~c_cable_cards()=default;

		/// return card for given cable type, this (reference) can be invalided on next call, use immediatelly!
		/// Spawn it it if not created yet.
		c_cable_base_obj & get_card(t_cable_kind kind);

		unique_ptr<c_cable_base_obj> create_card(t_cable_kind); ///< factory for cards

	protected:
		map<t_cable_kind , unique_ptr<c_cable_base_obj>> m_cards;

		shared_ptr<c_asioservice_manager> m_asioservice_manager; ///< needed to create cables based on asio service

		shared_ptr<c_asioservice_manager> get_asioservice(); ///< creates if needed and returns an asioservice manager
};






