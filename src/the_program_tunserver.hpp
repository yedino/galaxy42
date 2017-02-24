
#include "libs0.hpp"

#include "the_program.hpp"

#include "tunserver.hpp"

/// The object of main program. Usually just one object should exist per the process (unless you know what you're doing)
class c_the_program_tunserver : public c_the_program {
	public:
		c_the_program_tunserver()=default;
		virtual ~c_the_program_tunserver()=default;

		virtual void options_create_desc() override; ///< prepare description/definition of the program options
		virtual void options_multioptions() override; ///< parse some special options that add more options (e.g. developer tests)
		virtual std::tuple<bool,int> options_commands_run() override; ///< run special commands given in command line; returns should we exit and with what exit-code

		virtual int main_execution() override; ///< enter the main execution of program - usually containing the main loop; Return the exit-code of it.

	protected:
		unique_ptr<c_tunserver> m_myserver_ptr; ///< object of my node

		int m_http_dbg_port=9080; ///< the http-debug port
};

