
#include <cable/base/cable_base_addr.hpp>
#include <cable/base/cable_base_obj.hpp>
#include <cable/asio_ioservice_manager.hpp>
#include <cable/kind.hpp>
#include <cable/selector.hpp>
#include <libs0.hpp>

c_card_selector::c_card_selector(t_cable_kind kind, unique_ptr<c_cable_base_addr> && my_addr)
: m_kind(kind), m_my_addr(std::move(my_addr))
{
	_check_input(m_my_addr.get()); // not-null
}

c_card_selector::c_card_selector(const c_card_selector & other)
: m_kind(other.m_kind),
m_my_addr(  make_unique<decltype(m_my_addr)::element_type>(UsePtr(other.m_my_addr)  ))
{
}

c_card_selector & c_card_selector::operator=(const c_card_selector & other) {
	m_kind = other.m_kind;
	auto new_my_addr = make_unique<decltype(m_my_addr)::element_type>( UsePtr(other.m_my_addr) ) ;
	m_my_addr = std::move( new_my_addr );
	return *this;
}

bool c_card_selector::operator==(const c_card_selector & other) const {
	if (! (this->m_kind == other.m_kind) ) return false;
	if (! ( UsePtr(this->m_my_addr) == UsePtr(other.m_my_addr) ) ) return false;
	return true;
}

bool c_card_selector::operator!=(const c_card_selector & other) const {
	return (!(*this == other));
}

bool c_card_selector::operator<(const c_card_selector & other) const {
	if (! (this->m_kind < other.m_kind) ) return true;
	if (! ( UsePtr(this->m_my_addr) < UsePtr(other.m_my_addr) ) ) return true;
	return false;
}

void c_card_selector::print(ostream & out) const {
	out << "{" << m_kind << ": " << UsePtr(m_my_addr) << "}";
}

ostream & operator<<(ostream & out, const c_card_selector & obj) {
	obj.print(out);
	return out;
}

