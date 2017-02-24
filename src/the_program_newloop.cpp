
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



	//	newloop_main( argt );
	return 0;
}

