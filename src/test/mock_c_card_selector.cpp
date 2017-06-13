#include "mock_c_card_selector.hpp"

mock::mock_c_card_selector::mock_c_card_selector(unique_ptr<c_cable_base_addr> &&my_addr)
:
	m_my_addr(std::move(my_addr))
{
}

mock::mock_c_card_selector::mock_c_card_selector(const mock::mock_c_card_selector &rhs)
: m_my_addr( UsePtr(rhs.m_my_addr).clone() )
{
}

mock::mock_c_card_selector &mock::mock_c_card_selector::operator=(const mock::mock_c_card_selector &rhs) {
	if (this == &rhs) return *this; // self
	m_my_addr = rhs.m_my_addr->clone();
	return *this;
}

