
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

		/// Functions to apply/run various groups of options, settings
		/// @{
		virtual void use_options_peerref(); ///< this executs options that add peer references (like "--peer" / "peer")
		/// @}

	protected:
		bool m_pimpl_deleted = false; ///< assert: was delete yet called on pimpl? to avoid deleting it twice by mistake
		c_the_program_newloop_pimpl * const pimpl;
};

