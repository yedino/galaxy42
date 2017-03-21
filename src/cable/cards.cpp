
#include "cards.hpp"
#include "libs1.hpp"

#include "kind.hpp"

#include "simulation/cable_simul_obj.hpp"
#include "shm/cable_shm_obj.hpp"
#include "udp/cable_udp_obj.hpp"


unique_ptr< c_cable_base_obj > c_cable_cards::create_card(t_cable_kind kind) {
	switch (kind) {
		/*
		case e_cable_kind_simul:
			return make_unique<c_cable_simul_obj>();
		break;
		case e_cable_kind_shm:
			return make_unique<c_cable_shm_obj>();
		break;
		*/
		case e_cable_kind_udp:
			return make_unique<c_cable_udp>(get_asioservice());
		break;
		default:
		break;
	}
	_throw_error_runtime("unsupported cable kind");
}

shared_ptr<c_asioservice_manager> c_cable_cards::get_asioservice() {
	if (m_asioservice_manager == nullptr) {
		_note("Need to allocate asioservice manager");
		m_asioservice_manager = make_shared< c_asioservice_manager >( 4 ); // TODO option - ioservices
	}
	_check( m_asioservice_manager );
	return m_asioservice_manager;
}

c_cable_base_obj & c_cable_cards::get_card(t_cable_kind kind) {
	auto found = m_cards.find(kind);
	if (found == m_cards.end()) {
		_note("Create card for cable kind=" << static_cast<int>(kind));
		unique_ptr<c_cable_base_obj> card = c_cable_cards::create_card(kind);
		_note("Created card at " << static_cast<void*>(card.get()));
		m_cards.emplace( kind , std::move(card) );
		return UsePtr( m_cards.at(kind) );
	}
	return UsePtr( found->second );
}

void c_cable_cards::stop() {
	_note("Stopping all cards, size="<<m_cards.size());
	for(auto & cable_map_element : this->m_cards) {
		auto & ptr = cable_map_element.second;
		if (ptr) { UsePtr(ptr).stop(); } else _info("Element was null");
	}

	if (m_asioservice_manager) {
		UsePtr(m_asioservice_manager).stop_all();
	} else _info("ioservicemanager was null");
}

