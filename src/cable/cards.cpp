
#include "cards.hpp"
#include "libs1.hpp"

#include "kind.hpp"

#include "simulation/cable_simul_obj.hpp"
#include "shm/cable_shm_obj.hpp"
#include "udp/cable_udp_obj.hpp"
#include "udp/cable_udp_addr.hpp"


unique_ptr< c_cable_base_obj > c_cable_cards::create_card(const c_card_selector & selector) {
	_note("Creating cablecard, selector="<<selector);
	switch (selector.get_kind()) {
		/*
		case e_cable_kind_simul:
			return make_unique<c_cable_simul_obj>();
		break;
		case e_cable_kind_shm:
			return make_unique<c_cable_shm_obj>();
		break;
		*/
		case t_cable_kind::kind_udp:
		{
			return make_unique<c_cable_udp>(get_asioservice(), selector );
		}
		break;
		default:
		break;
	}
	_throw_error_runtime("unsupported cable kind");
}

shared_ptr<c_asioservice_manager_base> &c_cable_cards::get_asioservice() {
	if (m_asioservice_manager == nullptr) {
		_note("Need to allocate asioservice manager");
		m_asioservice_manager = make_shared< c_asioservice_manager >( 1 ); // TODO option - ioservices
	} else _dbg1("Returning existing get_asioservice");
	_check( m_asioservice_manager );
	return m_asioservice_manager;
}

c_cable_base_obj & c_cable_cards::get_card(const c_card_selector & selector) {
	_dbg1("get_card for selector="<<selector);
	auto found = m_cards.find(selector);
	if (found == m_cards.end()) {
		_note("Create card for cable kind=" << static_cast<int>(selector.get_kind()) << ", selector=" << selector);
		unique_ptr<c_cable_base_obj> card = c_cable_cards::create_card(selector);
		_note("Created card at " << static_cast<void*>(card.get()));
		m_cards.emplace( selector , std::move(card) );
		_dbg1("Emplaced, size: " << m_cards.size());
		_check( ! ( selector < selector ) ); // there was a bug
		return UsePtr( m_cards.at( selector ) );
	}
	_dbg2("get_card for selector="<<selector<<" - already existing");
	return UsePtr( found->second );
}

void c_cable_cards::stop_threadsafe() {
	_note("Stopping all cards, size="<<m_cards.size());
	for(auto & cable_map_element : this->m_cards) {
		auto & ptr = cable_map_element.second;
		if (ptr) { UsePtr(ptr).stop_threadsafe(); } else _info("Element was null");
	}

	if (m_asioservice_manager) {
		UsePtr(m_asioservice_manager).stop_all_threadsafe();
	} else _info("ioservicemanager was null");
}

