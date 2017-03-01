
#include "libs0.hpp"

#include "the_program.hpp"
#include "the_program_tunserver.hpp"

class c_the_program_newloop_pimpl;

/// The object of main program. Usually just one object should exist per the process (unless you know what you're doing)
class c_the_program_newloop : public c_the_program_tunserver {
	public:
		c_the_program_newloop();
		virtual ~c_the_program_newloop();

		virtual int main_execution() override; ///< enter the main execution of program - usually containing the main loop; Return the exit-code of it.

	protected:
		bool m_pimpl_deleted = false; ///< assert: was delete yet called on m_pimpl? to avoid deleting it twice by mistake
		c_the_program_newloop_pimpl * const m_pimpl;
};

