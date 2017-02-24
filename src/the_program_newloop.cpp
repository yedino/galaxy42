
#include "the_program_newloop.hpp"
#include "libs1.hpp"

#include "project.hpp"
#include <mo_reader.hpp>
#include "ui.hpp"
#include "trivialserialize.hpp"
#include "datastore.hpp"
#include "c_json_genconf.hpp"
#include "c_json_load.hpp"

#include "rpc/rpc.hpp"
#include "galaxy_debug.hpp"

#include "newloop.hpp"


#ifdef HTTP_DBG
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include "httpdbg/httpdbg-server.hpp"
#endif

// ============================================================================

atomic<int> readtun_nr;
size_t readtun( char * buf , size_t bufsize ) { // semantics like "read" from C.
	int x = readtun_nr++;
	string d = "foo"s;
	auto size_full = d.size();
	_check(size_full <= bufsize); // we must fit in buffer
	std::copy( buf , d.c_str() , bufsize );
	return size_full;
}

class c_tunserver2 {
	public:
		vector< std::thread > m_tun_reader;
};

class c_the_program_newloop_pimpl {
	public:
		c_the_program_newloop_pimpl()=default;
		~c_the_program_newloop_pimpl()=default;

		unique_ptr<c_tunserver2> tunserver;

	private:
		friend class c_the_program_newloop;
};


// -------------------------------------------------------------------

template <typename T>
struct c_to_report {
	public:
		const T & m_obj;
		int m_level;
		c_to_report(const T & obj, int level) : m_obj(obj), m_level(level) {}
};

template <typename TS, typename TR> TS & operator<<(TS & ostr , const c_to_report<TR> & to_report) {
	to_report.m_obj.report(ostr, to_report.m_level);
	return ostr;
}

template <typename T>
const c_to_report<T> make_report(const T & obj, int level) {
	return c_to_report<T>( obj , level);
}

// -------------------------------------------------------------------

class c_netchunk {
	public:
		typedef unsigned char t_element; ///< type of one elemenet

		c_netchunk(t_element * _data, size_t _size);

	public:
		t_element * const data; // points to inside of some existing t_netbuf. you do *NOT* own the data.
		const size_t size;
};

// -------------------------------------------------------------------

/***
	@brief Gives you a buffer of continous memory of type ::t_element (octet - unsigned char) with minimal API
*/
class c_netbuf {
	public:
		typedef c_netchunk::t_element t_element; ///< type of one elemenet

		c_netbuf(size_t size); ///< construct and allocate

		size_t size() const;
		// vector<t_element> & get_data(); ///< access data
		// const vector<t_element> & get_data() const; ///< access data
		t_element & at(size_t ix); ///< access one element (asserted)

		void report(std::ostream & ostr, int detail) const;

	private:
		vector<unsigned char> m_data; ///< my actuall data storage
};

c_netbuf::c_netbuf(size_t size) : m_data(size) {
	_dbg1( make_report(*this,10) );
}

size_t c_netbuf::size() const {	return m_data.size(); }

c_netbuf::t_element & c_netbuf::at(size_t ix) { return m_data.at(ix); }

void c_netbuf::report(std::ostream & ostr, int detail) const {
	ostr << "this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_data.size()
		<< ",memory@" << static_cast<const void*>(m_data.data()) ;
}

// -------------------------------------------------------------------

// ============================================================================

c_the_program_newloop::c_the_program_newloop()
	: m_pimpl( new c_the_program_newloop_pimpl() )
{	}

c_the_program_newloop::~c_the_program_newloop() {
	_check( ! m_pimpl_deleted); // avoid double destruction of this object
	m_pimpl_deleted=true;
	if (m_pimpl) { delete m_pimpl; }
}

int c_the_program_newloop::main_execution() {
	_mark("newloop main");
	m_pimpl->tunserver = make_unique< c_tunserver2 >();

	c_netbuf(9000);
	readtun();

	//	newloop_main( argt );
	return 0;
}

