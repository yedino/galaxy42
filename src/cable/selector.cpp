
#include <cable/base/cable_base_addr.hpp>
#include <cable/base/cable_base_obj.hpp>
#include <cable/asio_ioservice_manager.hpp>
#include <cable/kind.hpp>
#include <cable/selector.hpp>
#include <libs0.hpp>

c_card_selector::c_card_selector(unique_ptr<c_cable_base_addr> && my_addr)
: m_my_addr(std::move(my_addr))
{
	_check_abort(m_my_addr.get()); // not-null
}

c_card_selector::c_card_selector(const c_card_selector & other)
: m_my_addr( UsePtr(other.m_my_addr).clone() )
{
}

c_card_selector & c_card_selector::operator=(const c_card_selector & other) {
	if (this == &other) return *this; // self
	m_my_addr = other.m_my_addr->clone();
	return *this;
}

bool c_card_selector::operator==(const c_card_selector & other) const {
	if (! ( UsePtr(this->m_my_addr) == UsePtr(other.m_my_addr) ) ) return false;
	return true;
}

bool c_card_selector::operator!=(const c_card_selector & other) const {
	return (!(*this == other));
}

bool c_card_selector::operator<(const c_card_selector & other) const {
	if (  ( UsePtr(this->m_my_addr) < UsePtr(other.m_my_addr) ) ) return true;
	return false;
}

void c_card_selector::print(ostream & out) const {
	out << "{" << UsePtr(m_my_addr) << "}";
}

ostream & operator<<(ostream & out, const c_card_selector & obj) {
	obj.print(out);
	return out;
}

