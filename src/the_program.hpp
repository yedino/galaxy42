
#pragma once

#include "libs0.hpp"
#include <boost/program_options.hpp>

/// The object of main program. Usually just one object should exist per the process (unless you know what you're doing)
class c_the_program {
	public:
		c_the_program()=default;
		virtual ~c_the_program()=default;

		virtual void take_args(int argc, const char **argv); ///< set the argc,argv
		virtual void take_args(const string & _argt_exec , const vector<string> & _argt); ///< sets exec-name, and rest of args

		virtual void startup_console_first(); ///< program should detect environment for console (e.g. are color-codes ok)
		virtual void startup_version(); ///< show basic info about version
		virtual void startup_data_dir(); ///< find the data dir, set install_dir_base
		virtual void startup_locales_early(); ///< setup locales, e.g. mo_file_reader - very early, to have some standard locales
		virtual void startup_locales_later(); ///< setup locales, e.g. mo_file_reader - a bit later, when e.g. debugging works normally
		virtual void startup_curl(); ///< prepares library curl

		virtual void init_library_sodium(); ///< init library

		/// run special tests instead of main program. Returns: {should-we-exit, error-code-if-we-exit}
		virtual std::tuple<bool,int> program_startup_special();

		virtual void options_create_desc(); ///< prepare description/definition of the program options
		virtual void options_parse_first(); ///< parse the command-line options
		virtual void options_multioptions(); ///< parse some special options that add more options (e.g. developer tests)
		virtual void options_done(); ///< done parsing of options+multioptions

		std::tuple<bool,int> options_commands_run(); ///< see base_options_commands_run()

		virtual int main_execution(); ///< enter the main execution of program - usually containing the main loop; Return the exit-code of it.

		virtual bool check_and_remove_special_cmdline(const string & name); ///< checks if given command-line argument is given, if yes then remove it and return true

	protected:


		/// Partiall execution - call it from public functions, possible re-use it in child if you want
		virtual std::tuple<bool,int> base_options_commands_run(); ///< run special commands given in command line; returns should we exit and with what exit-code

		/// Raw command-line options
		///@{
		vector<string> argt; ///< the arguments with which the program is running, except for program name (see argt_exec)
		string argt_exec; ///< the name of running binary program (like from argv[0])
		///@}

		/// Boost program-options (for options from command-line and more)
		///@{
		boost::program_options::variables_map m_argm; ///< the arguments map (parsed, ready to use)
		shared_ptr<boost::program_options::options_description> m_boostPO_desc; ///< description/definition of the possible options, for parsing
		///@}

		/// Dirs/path options for the program
		/// @{
		string m_install_dir_base; ///< here we will find main dir like "/usr/" that contains our share dir, it will be terminated with "/", or it will be an empty string "".
		std::string m_install_dir_share_locale; ///< dir with our locale data, will be terminated with "/", or will be an empty string ""
		/// @}

		/// Debug options for the program
		///@{
		static constexpr int config_default_basic_dbg_level = 60; // [debug] level default
		static constexpr int config_default_incrased_dbg_level = 20; // [debug] early-debug level if user used --d
		///@}

		string config_default_myname;
};

// show title at start of main sections:
#define PROGRAM_SECTION_TITLE pfp_goal("\n======================================================================\n" \
	<< "Entering: " << __func__ )


