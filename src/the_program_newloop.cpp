
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


class c_the_program_newloop_pimpl {
	public:
		c_the_program_newloop_pimpl()=default;
		~c_the_program_newloop_pimpl()=default;

	private:
		friend class c_the_program_newloop;
};

// ============================================================================

c_the_program_newloop::c_the_program_newloop()
	: m_pimpl( new c_the_program_newloop_pimpl() )
{ }

c_the_program_newloop::~c_the_program_newloop() {
	_check( ! m_pimpl_deleted); // avoid double destruction of this object
	m_pimpl_deleted=true;
	if (m_pimpl) { delete m_pimpl; }
}

int c_the_program_newloop::main_execution() {
	newloop_main( argt );
	return 0;
}

