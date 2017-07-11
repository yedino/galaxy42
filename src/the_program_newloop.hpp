
#include "libs0.hpp"

#include "the_program.hpp"
#include "the_program_tunserver.hpp"

class c_the_program_newloop_pimpl;

/// The object of main program. Usually just one object should exist per the process (unless you know what you're doing)
/// This is the new-loop (with threads etc)
class c_the_program_newloop : public c_the_program_tunserver {
	public:
		c_the_program_newloop();
		virtual ~c_the_program_newloop();

		virtual int main_execution() override; ///< enter the main execution of program - usually containing the main loop; Return the exit-code of it.

		/// Functions to apply/run various groups of options, settings
		/// @{
		virtual void use_options_peerref(); ///< this executs options that add peer references (like "--peer" / "peer")
		/// @}

		/// Main tasks of the program. Also @see main_execution()
		/// @{
		void programtask_load_my_keys();
		void programtask_tuntap(); ///< start tuntap
		/// @}

		int run_special(); ///< run special commands (eg --special-ubsan1); if returns non-zero then program must abort with this exit-code

	protected:
		bool m_pimpl_deleted = false; ///< assert: was delete yet called on pimpl? to avoid deleting it twice by mistake
		c_the_program_newloop_pimpl * const pimpl;

		std::tuple<bool,int> base_options_commands_run() override;
		std::tuple<bool,int> programtask_help(const string & topic);

		FRIEND_TEST(the_program_new_loop, use_options_peerref);
};


///@param level - memory use allowed: level=0 - basic tests,  level=1 also SIDH, level=2 also NTru
void test_create_cryptolink(const int number_of_test, int level=0);
