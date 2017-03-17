
#include "netbuf.hpp"
#include "tnetdbg.hpp"

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

// -------------------------------------------------------------------


c_netbuf::c_netbuf(size_t size) {
	_dbg2("allocating");
	m_data = new t_element[size]; // fast new - no initialization of data
	m_size = size;
	_dbg1( make_report(*this,10) );
}

c_netbuf::~c_netbuf() {
	_dbg1("dealloc: " << make_report(*this,10) );
	delete[] m_data;
	m_data=nullptr;
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

// -------------------------------------------------------------------

