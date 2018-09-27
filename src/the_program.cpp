
#include "libs1.hpp"

#include "the_program.hpp"

#include <boost/filesystem.hpp>
#include "glue_sodiumpp_crypto.hpp"

#include <clocale>


#ifdef ENABLE_LIB_CURL
	#include <curl/curl.h>
#endif

#include "bitcoin_node_cli.hpp"

void c_the_program::take_args(int argc, const char **argv) {
	if (argc>0) argt_exec=argv[0]; else argt_exec="";
	for (int i=1; i<argc; ++i) argt.push_back(argv[i]);
}

void c_the_program::take_args(const string & _argt_exec , const vector<string> & _argt) {
	argt_exec = _argt_exec;
	argt = _argt;
}


void c_the_program::startup_console_first() {
}

/// Show program version
void c_the_program::startup_version() {
	ostringstream oss; oss << "ver. "
		<< project_version_number_major << "."
		<< project_version_number_minor << "."
		<< project_version_number_sub << "."
		<< project_version_number_progress // << "."
		<< project_version_number_patch
		#ifdef TOOLS_ARE_BROKEN // this is passed by CMake
		<< " [broken-tools] "
		#endif
		;
	string ver_str = oss.str();
	pfp_fact( "" ); // newline
	pfp_fact( "Start... " << ver_str );
	#ifdef TOOLS_ARE_BROKEN
		pfp_clue("[broken-tools] it seems this program was built with partially broken tools/libraries "
			"(details are shown when building, e.g. by cmake/ccmake)");
	#endif
}

bool c_the_program::check_and_remove_special_cmdline(const string & name) {
	if (contains_value(argt, name)) {
		while (true) { // keep erasing all occurances
			auto it = find(argt.begin() , argt.end(), name);
			if (it == argt.end()) break; // not found
			argt.erase(it);
		}
		return true;
	}
	return false;
}

std::tuple<bool,int> c_the_program::program_startup_special() {
	return std::tuple<bool,int>(false, 0); // tell it to continue
}

void c_the_program::startup_data_dir() {
	std::cerr << "Start: " << __func__ << std::endl;
	pfp_fact("in " << __func__);
	bool found=false;
	try
	{
		namespace b_fs = boost::filesystem;
		// find path to my main data dir (to my data "in share").
		// e.g. /home/rafalcode/work/galaxy42/    - because it contains:
		//      /home/rafalcode/work/galaxy42/share/locale/en/LC_MESSAGES/galaxy42_main.mo

		pfp_fact("Current path [" << b_fs::current_path() << "]");
		b_fs::path cwd_full_boost( b_fs::current_path() );
		auto cwd_full = b_fs::canonical( b_fs::absolute( cwd_full_boost ) );
		// string cwd_full = b_fs::lexically_norma( b_fs::absolute( cwd_full_boost ) ).string();

		b_fs::path selfpath = "";
		selfpath += argt_exec; // (to include any extra path to binary) file name will be removed below leaving just path
		//pfp_fact("selfpath (2) = [" << selfpath << "]");
		b_fs::path selfdir_boost = selfpath.remove_filename();
		auto selfdir = b_fs::canonical( b_fs::absolute( selfdir_boost  ));
		pfp_fact("selfdir = [" << selfdir << "]");

		std::cerr << "Start... [" << cwd_full << "] (cwd) " << std::endl;
		std::cerr << "Start... [" << selfdir << "] (exec) " << std::endl;

		const string toplevel_marker_filename = "toplevel-project-yedino-org"; // name of file, that will be present in top level of source code, to mark this project [project] [yedino]
		const string toplevel_example_file = "share/locale/en/LC_MESSAGES/galaxy42_main.mo"; // if this file exists then this looks like a good directory

		vector<string> data_dir_possible; // dirs (relative, or absolute) that might contain data dir including translations
		std::set<string> data_dir_needs_marker; // put here dirs from like in data_dir_possible[] that require to have toplevel_marker_filename file

		data_dir_possible.push_back(cwd_full.string() + "/");
		data_dir_possible.push_back(selfdir.string() + "/");

		#if defined(__MACH__)
			// TODO when macosx .dmg fully works (when Gitian on macosx works)
		#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER)
			// data files should be next to .exe
		#else
			data_dir_possible.push_back("/usr/");
			data_dir_possible.push_back("/usr/local/");
			data_dir_possible.push_back("/usr/opt/");

			{ const string d="../";	// in case when we are in build directory right under source root
				data_dir_possible.push_back(d);
				data_dir_needs_marker.insert(d); // this dir must have a marker file there, since it's some random location "../" might end up in unexpected files of user
			}
		#endif

		for (auto && dir : data_dir_possible) {
			if (data_dir_needs_marker.count(dir)) { // does this dir need a marker-file before we start poking around
				const string testname = dir + toplevel_marker_filename; // if this file exists then this looks like a good directory
				pfp_fact( "Test? [" << testname << "]... " );
				ifstream filetest( testname.c_str() );
				if (! filetest.good()) continue; // !
			}
			const string testname = dir + toplevel_example_file; // if this file exists then this looks like a good directory
			pfp_fact( "Test: [" << testname << "]... " );
			ifstream filetest( testname.c_str() );
			if (filetest.good()) {
				m_install_dir_base = dir;
				found=true;
				pfp_fact("OK ");
				break;
			}
		}
	} catch(std::exception & ex) {
			pfp_erro( "Error while looking for data directory ("<<ex.what()<<")" << std::endl );
	}

	if (!found) {
		pfp_fact( "Can not find language data files." );
		assert( m_install_dir_base == "" );
	}
	else {
		assert( at_back(m_install_dir_base)=='/' );
		pfp_fact( "Data: [" << m_install_dir_base << "]" );
		m_install_dir_share_locale = m_install_dir_base + "share/locale/";
		pfp_fact( "Lang: [" << m_install_dir_share_locale << "]" );
	}
}

void c_the_program::startup_locales_early() {
	try {
		const char * const result = std::setlocale(LC_ALL, "C"); // save to char* instead duplicate to string, so that we can test it versus NULL
		if (result == nullptr) throw std::runtime_error("Can not set locale");
	} catch (...) {
		std::cerr << "Error: setlocale" << std::endl;
	}
}


void c_the_program::startup_locales_later() {
	{
		pfp_fact("Locale switch to user...");
		const char * const locale_ptr = setlocale(LC_ALL,"");
		string locale_str( locale_ptr ); // copy the c-string while it's valid!
		if (locale_ptr == nullptr) {
			pfp_fact("Locale switch to user: error... will use C locale");
			const char * const locale_fix_ptr = setlocale(LC_ALL,"C");
			if (locale_fix_ptr == nullptr) throw std::runtime_error("Can not set user locale, and also can not switch to safe C locale.");
		} else {
			pfp_fact("Locale switch to user: OK. Old locale: [" << locale_str << "]");
		}
	}

	{
		const char * const locale_ptr = setlocale(LC_ALL,"");
		string locale_str( locale_ptr ); // copy the c-string while it's valid!
		if (locale_ptr == nullptr) { // should be unused since above checks
			pfp_fact("Locale switch to user: error (and can not fix it)");
			throw std::runtime_error("Can not set user locale (nor fix it)");
		}
		pfp_fact("Using locale: [" << locale_str << "]");
	}

/*

This was an old attempt to just use normal gettext and locale,
but it was not working on some platforms (afair MSVC) ; by @rob ? --rfree
Leaving as documentation/example

	boost::locale::generator gen;
	// Specify location of dictionaries
	gen.add_messages_path(m_install_dir_share_locale);
	gen.add_messages_domain("galaxy42_main");
	std::string locale_name;
	try {
		locale_name = std::use_facet<boost::locale::info>(gen("")).name();
		std::cerr << "Locale: " << locale_name << endl;
	} catch (const std::exception &e) {
		std::cerr << "Can not detect language, set default language" << "\n";
		locale_name = "en_US.UTF-8";
	}
	std::locale::global(gen(locale_name));
	//std::locale::global(gen("pl_PL.UTF-8")); // OK
	//std::locale::global(gen("Polish_Poland.UTF-8")); // does not work
	std::cout.imbue(std::locale());
	std::cerr.imbue(std::locale());
	// Using mo_file_reader::gettext:
	std::cerr << std::string(80,'=') << std::endl << mo_file_reader::gettext("L_warning_work_in_progres") << std::endl << std::endl;
	std::cerr << mo_file_reader::gettext("L_program_is_pre_pre_alpha") << std::endl;
	std::cerr << mo_file_reader::gettext("L_program_is_copyrighted") << std::endl;
	std::cerr << std::endl;
*/
	try {
		mo_file_reader mo_reader;
		pfp_fact("Adding MO for shared dir [" << m_install_dir_share_locale << "]");
		mo_reader.add_messages_dir(m_install_dir_share_locale);
		mo_reader.add_mo_filename(std::string("galaxy42_main"));
		mo_reader.read_file();
	} catch (const std::exception &e) {
		pfp_erro( "mo file open error: " << e.what() );
	}

	pfp_fact( std::string(80,'=') << std::endl << mo_file_reader::gettext("L_warning_work_in_progres") << std::endl );
	pfp_fact( mo_file_reader::gettext("L_program_is_pre_pre_alpha") );
	pfp_fact( mo_file_reader::gettext("L_program_is_copyrighted") );

//	const std::string m_install_dir_share_locale="share/locale"; // for now, for running in place
//	setlocale(LC_ALL,"");
//	string used_domain = bindtextdomain ("galaxy42_main", m_install_dir_share_locale.c_str() );
//	textdomain("galaxy42_main");
	// Using mo_file_reader::gettext:
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_pre_pre_alpha") << std::endl;
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_copyrighted") << std::endl;
}

void c_the_program::startup_curl() {
	#ifdef ENABLE_LIB_CURL
	// This curl init code MUST be 1-thread and very early in main
	CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(res != CURLE_OK) {
		bitcoin_node_cli::curl_initialized=false;
		std::cerr<<"Error: lib curl init."<<std::endl;
	}
	else{
		bitcoin_node_cli::curl_initialized=true;
	}
	#else
		// initialize of the fake curl that does nothing
		bitcoin_node_cli::curl_initialized=true;
	#endif
}

void c_the_program::init_library_sodium() {
	pfp_fact(mo_file_reader::gettext("L_starting_lib_libsodium"));

	if (sodium_init() == -1) {
		pfp_throw_error( std::runtime_error(mo_file_reader::gettext("L_lisodium_init_err")) );
	}
	pfp_info(mo_file_reader::gettext("L_libsodium_ready"));
}

void c_the_program::options_create_desc() { }

void c_the_program::options_parse_first() {
	pfp_goal("Will parse commandline, got args count: " << argt.size() << " and exec="<<argt_exec );
	for (const auto & str : argt)
	{
		pfp_fact("commandline option: " << str << " ;");
		if (str.size() == 0) pfp_warn("Empty commandline arg");
	}
	_check(m_boostPO_desc);
	namespace po = boost::program_options;

	// back to argc/argv, so that boost::program_options can parse it
	c_string_string_Cstyle args_cstyle( argt_exec , argt );
	const int argc = args_cstyle.get_argc();
	const char ** argv = args_cstyle.get_argv();

	po::store(po::parse_command_line(argc, argv, *m_boostPO_desc) , m_argm); // *** parse commandline, and store result
	pfp_dbg1( "Parsing with options: " << *m_boostPO_desc );
	pfp_goal("BoostPO parsed argm size=" << m_argm.size());
	for(auto &arg: m_argm) pfp_info("Argument in argm: " << arg.first );
}

void c_the_program::options_multioptions() { }

std::tuple<bool,int> c_the_program::base_options_commands_run() {
	return std::tuple<bool,int>(false,0);
}

std::tuple<bool,int> c_the_program::options_commands_run() {
	PROGRAM_SECTION_TITLE;
	return this->base_options_commands_run();
}

void c_the_program::options_done() {
			// === argm now can contain options added/modified by developer mode ===
			namespace po = boost::program_options;
			po::notify(m_argm);  // !
			pfp_note("After BoostPO notify");

	for (const auto & opt : m_argm) {
		ostringstream oss ; oss << "commandline option (parsed by boost PO): " << opt.first << " = ";
		bool converted=false;
		#define try_convert_as(TYPE) \
			try { ostringstream oss2; oss2 << "(" << #TYPE << ") " << opt.second.as<TYPE>(); \
			converted=1; oss<<oss2.str(); } \
			catch(const boost::bad_any_cast &) {}
		try_convert_as(string);
		try_convert_as(int);
		try_convert_as(bool);
		#undef try_convert_as

		if (!converted) oss << "(other type)";
		oss << " ;";
		pfp_note(oss.str());
	}
}

int c_the_program::main_execution() {
	return 0;
}
