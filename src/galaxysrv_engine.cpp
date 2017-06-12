
#include "galaxysrv_engine.hpp"


c_weld::c_weld(size_t memsize)
:
m_empty(true),
// Init here things that are documented in m_empty as always valid:
m_buf( memsize )
{
	clear();
}

void c_weld::clear() {
	m_empty=true;
	m_bufRead=0;
	m_bufWrite=0;
}

size_t c_weld::get_free_size() const {
	return eint::eint_minus( m_buf.size() , m_bufWrite );
}

void c_weld::adjust_bufWrite(size_t new_bufWrite) {
	_check( new_bufWrite <= m_buf.size() );
	m_bufWrite = new_bufWrite;
}

