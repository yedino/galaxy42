#include "netbuf.hpp"
#include "tnetdbg.hpp"


c_netchunk::c_netchunk(c_netchunk && rhs) noexcept
: m_data(rhs.m_data), m_size(rhs.m_size) {
	rhs.m_data = nullptr;
	rhs.m_size = 0;
}

c_netchunk::c_netchunk(t_element * data, size_t size) : m_data(data), m_size(size) { }

void c_netchunk::report(std::ostream & ostr, int detail) const {
	ostr << "netchunk this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_size
		<< ",memory@" << static_cast<const void*>(m_data) ;
	if (detail>=20) {
		ostr << " [";
		for (size_t i=0; i<m_size; ++i) {
			if (i) ostr<<' ';
			ostr << std::hex << static_cast<int>(m_data[i]) << std::dec ;
		}
		ostr << "]";
	}
}

size_t c_netchunk::size() const noexcept { return m_size; }
c_netchunk::t_element * c_netchunk::data() const noexcept{ return m_data; }

stdplus::tab_view<unsigned char> c_netchunk::to_tab_view() const {
	return stdplus::tab_view<unsigned char>( m_data, m_size );
}

void c_netchunk::shrink_to(size_t smaller_size) {
	_check(smaller_size <= m_size);
	m_size = smaller_size;
}

// -------------------------------------------------------------------


c_netbuf::c_netbuf(c_netbuf && brother) noexcept
: m_data(brother.m_data), m_size(brother.m_size)
{
	brother.m_data=nullptr;
	brother.m_size=0;
}

c_netbuf & c_netbuf::operator=(c_netbuf && brother) noexcept {
	if (& brother == this) return *this; // self-assigment
	m_data = brother.m_data;
	m_size = brother.m_size;
	brother.m_data=nullptr;
	brother.m_size=0;
	return *this;
}

c_netbuf::c_netbuf(size_t size)
: m_data(nullptr), m_size(0)
{
	try { // we must make sure m_data is deleted, because we do not use unique_ptr here
		_dbg2("allocating");
		m_data = new t_element[size]; // fast new - no initialization of data
		// m_data = make_unique<t_element[]>(size);
		m_size = size;
		_dbg1( make_report(*this,10) );
	} catch(...) {
		clear();
		throw ;
	}
}

c_netbuf::~c_netbuf() {
	clear();
}

void c_netbuf::clear() {
	_dbg1("dealloc: " << make_report(*this,10) );
	if (m_data) {
		delete[] m_data;
		m_data=nullptr;
	}
	m_size=0;
}

size_t c_netbuf::size() const noexcept { return m_size; }

c_netbuf::t_element & c_netbuf::at(size_t ix) {
	_check_user(2);
	_check_input(ix<m_size);
	return  *( m_data + ix );
}

void c_netbuf::report(std::ostream & ostr, int detail) const {
	ostr << "netBUF this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_size
		<< ",memory@" << static_cast<const void*>(m_data) ;
	if (detail>=20) {
		ostr << " [";
		for (size_t i=0; i<m_size; ++i) {
			if (i) ostr<<' ';
			ostr << std::hex << static_cast<int>(m_data[i]) << std::dec ;
		}
		ostr << "]";
	}
}

c_netbuf::t_element * c_netbuf::data() noexcept { return m_data; }

c_netbuf::t_element const * c_netbuf::data() const noexcept { return m_data; }

stdplus::tab_view<unsigned char> c_netbuf::to_tab_view() const {
	return stdplus::tab_view<unsigned char>( m_data, m_size );
}

c_netchunk c_netbuf::get_chunk(size_t offset, size_t size) {
	_check_abort( eint::eint_plus( offset , size ) <= m_size );
	return c_netchunk(m_data + offset , size);
}

// -------------------------------------------------------------------

