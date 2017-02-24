
#include "libs1.hpp"

#include "the_program.hpp"

#include <boost/filesystem.hpp>
#include "glue_sodiumpp_crypto.hpp"


c_the_program::c_the_program() { }

void c_the_program::take_args(int argc, const char **argv) {
	if (argc>0) argt_exec=argv[0]; else argt_exec="";
	for (int i=1; i<argc; ++i) argt.push_back(argv[i]);
}

void c_the_program::take_args(const string & _argt_exec , const vector<string> & _argt) {
	argt_exec = _argt_exec;
	argt = _argt;
}

c_the_program::~c_the_program() { }

void c_the_program::startup_console_first() {
}

/// Show program version
void c_the_program::startup_version() {
	ostringstream oss; oss << "ver. "
		<< project_version_number_major << "."
		<< project_version_number_minor << "."
		<< project_version_number_sub // << "."
		<< project_version_number_patch ;
	string ver_str = oss.str();
	_fact( "Start... " << ver_str );
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
	bool found=false;
	try
	{
		namespace b_fs = boost::filesystem;
		// find path to my main data dir (to my data "in share").
		// e.g. /home/rafalcode/work/galaxy42/    - because it contains:
		//      /home/rafalcode/work/galaxy42/share/locale/en/LC_MESSAGES/galaxy42_main.mo

		// we could normalize the path... but this could trigger more problems maybe with encoding of string.
		auto dir_normalize = [](std::string path) -> std::string {
			return path; // nothing for now. TODO (would be nicer to not display "//home/.." in this tests below
		};

		b_fs::path cwd_full_boost( b_fs::current_path() );
		string cwd_full = dir_normalize( b_fs::absolute( cwd_full_boost ) .string() );
		// string cwd_full = b_fs::lexically_norma( b_fs::absolute( cwd_full_boost ) ).string();

		b_fs::path selfpath = "";
		// += cwd_full_boost;
		// selfpath += "/";
		selfpath += argt_exec; // hmm, what? --rafal
		b_fs::path selfdir_boost = selfpath.remove_filename();
		string selfdir = dir_normalize( b_fs::absolute( selfdir_boost ) .string() );

//		cerr << "Start... [" << cwd_full << "] (cwd) " << endl;
//		cerr << "Start... [" << selfdir << "] (exec) " << endl;

		vector<string> data_dir_possible;
		data_dir_possible.push_back(cwd_full);
		data_dir_possible.push_back(selfdir);

		#if defined(__MACH__)
			// TODO when macosx .dmg fully works (when Gitian on macosx works)
		#elif defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER)
			// data files should be next to .exe
		#else
			data_dir_possible.push_back("/usr");
			data_dir_possible.push_back("/usr/local");
			data_dir_possible.push_back("/usr/opt");
		#endif

		for (auto && dir : data_dir_possible) {
			string testname = dir;
			testname += "/share/locale/en/LC_MESSAGES/galaxy42_main.mo";
			_fact( "Test: [" << testname << "]... " << std::flush );
			ifstream filetest( testname.c_str() );
			if (filetest.good()) {
				m_install_dir_base = dir;
				found=true;
				_fact(" OK! " );
				break;
			} else _fact( "" );
		}
	} catch(std::exception & ex) {
			_erro( "Error while looking for data directory ("<<ex.what()<<")" << std::endl );
	}

	if (!found) _fact( "Can not find language data files." );

	_fact( "Data: [" << m_install_dir_base << "]" );
	m_install_dir_share_locale = m_install_dir_base + "/share/locale";
	_fact( "Lang: [" << m_install_dir_share_locale << "]" );
}

void c_the_program::startup_locales() {
	setlocale(LC_ALL,"");

/*	boost::locale::generator gen;
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
	//std::locale::global(gen("Polish_Poland.UTF-8")); // not works
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
		_fact("Adding MO for" << m_install_dir_share_locale);
		mo_reader.add_messages_dir(m_install_dir_share_locale);
		mo_reader.add_mo_filename(std::string("galaxy42_main"));
		mo_reader.read_file();
	} catch (const std::exception &e) {
		_erro( "mo file open error: " << e.what() );
	}

	_fact( std::string(80,'=') << std::endl << mo_file_reader::gettext("L_warning_work_in_progres") << std::endl );
	_fact( mo_file_reader::gettext("L_program_is_pre_pre_alpha") );
	_fact( mo_file_reader::gettext("L_program_is_copyrighted") );
	_fact( "" );

//	const std::string m_install_dir_share_locale="share/locale"; // for now, for running in place
//	setlocale(LC_ALL,"");
//	string used_domain = bindtextdomain ("galaxy42_main", m_install_dir_share_locale.c_str() );
//	textdomain("galaxy42_main");
	// Using mo_file_reader::gettext:
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_pre_pre_alpha") << std::endl;
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_copyrighted") << std::endl;
}

void c_the_program::init_library_sodium() {
	_fact(mo_file_reader::gettext("L_starting_lib_libsodium"));

	if (sodium_init() == -1) {
		_throw_error( std::runtime_error(mo_file_reader::gettext("L_lisodium_init_err")) );
	}
	_info(mo_file_reader::gettext("L_libsodium_ready"));
}

void c_the_program::options_create_desc() { }

void c_the_program::options_parse_first() {
	_goal("Will parse commandline, got args count: " << argt.size() << " and exec="<<argt_exec );
	_check(m_boostPO_desc);
	namespace po = boost::program_options;
	c_string_string_Cstyle args_cstyle( argt_exec , argt );
	const int argc = args_cstyle.get_argc();
	const char ** argv = args_cstyle.get_argv();
	po::store(po::parse_command_line(argc, argv, *m_boostPO_desc) , m_argm); // parse commandline, and store result
	_note("BoostPO parsed argm size=" << m_argm.size());
}

void c_the_program::options_multioptions() { }
std::tuple<bool,int> c_the_program::options_commands_run() {
	return std::tuple<bool,int>(false,0);
}

void c_the_program::options_done() {
			// === argm now can contain options added/modified by developer mode ===
			namespace po = boost::program_options;
			po::notify(m_argm);  // !
			_note("After BoostPO notify");
			for(auto &arg: m_argm) _info("Argument in argm: " << arg.first );
}

int c_the_program::main_execution() {
	return 0;
}
