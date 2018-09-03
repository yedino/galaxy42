// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt
#include "the_program.hpp"
#include "the_program_tunserver.hpp"
#include "the_program_newloop.hpp"
#include "utils/privileges.hpp"
#include <clocale>

#include "utils/capmodpp.hpp" // to capture it's exceptions

#include "../src-tools/netmodel/netmodel.hpp"

bool run_mode_developer_main(boost::program_options::variables_map & argm);

namespace developer_tests {

std::string make_pubkey_for_peer_nr(int peer_nr) {
	std::string peer_pub = std::string("4a4b4c") + std::string("4") + std::string(1, char('0' + peer_nr)  );
	return peer_pub;
}

// list of peers that exist in our test-world:
struct t_peer_cmdline_ref {
	std::string ip;
	std::string pubkey;
	std::string privkey; ///< just in the test world. here we have knowledge of peer's secret key
};

bool wip_galaxy_route_star(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int node_nr = argm["develnum"].as<int>();  assert( (node_nr>=1) && (node_nr<=254) );
        pfp_fact( mo_file_reader::gettext("L_running_devel_as_node_nr") << node_nr );

	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");

	int peer_nr = node_nr==1 ? 2 : 1;

	std::string peer_pub = make_pubkey_for_peer_nr( peer_nr );
	// each connect to node .1., except the node 1 that connects to .2."
	std::string peer_ip = std::string("192.168.") + std::to_string( peer_nr  ) + std::string(".62");

//	pfp_mark("Developer: adding peer with arguments: ip=" << peer_ip << " pub=" << peer_pub );
        pfp_mark(mo_file_reader::gettext("L_devel_adding_peer_with_args") << "ip=" << peer_ip << " pub=" << peer_pub );

	// argm.insert(std::make_pair("K", po::variable_value( int(node_nr) , false )));
	argm.insert(std::make_pair("peerip", po::variable_value( peer_ip , false )));
	argm.at("peerpub") = po::variable_value( peer_pub , false );
	argm.at("mypub") = po::variable_value( make_pubkey_for_peer_nr(node_nr)  , false );
	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(node_nr) , false );
	return true; // continue the test
}

void add_program_option_vector_strings(boost::program_options::variables_map & argm,
	const std::string & name , const std::string & value_to_append)
{
	namespace po = boost::program_options;
	std::vector<std::string> old_peer;
	try {
		old_peer = argm[name].as<std::vector<std::string>>();
		old_peer.push_back(value_to_append);
		argm.at(name) = po::variable_value( old_peer , false );
	} catch(boost::bad_any_cast) {
		old_peer.push_back(value_to_append);
		argm.insert( std::make_pair(name , po::variable_value( old_peer , false )) );
	}
//	pfp_info("program options: added to option '" << name << "' - now size: " << argm.at(name).as<vector<string>>().size() );
        pfp_info(mo_file_reader::gettext("L_program_option_added") << name << mo_file_reader::gettext("L_option_now_size") << argm.at(name).as<std::vector<std::string>>().size() );

}

bool wip_galaxy_route_pair(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
//	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;
        pfp_fact( mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr );

	if (my_nr == 1) add_program_option_vector_strings(argm, "peer", "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b");
	if (my_nr == 2) add_program_option_vector_strings(argm, "peer", "192.168.1.62:9042-fd42:ae11:f636:8636:ae76:acf5:e5c4:dae1");

	return true;
}

bool demo_sodiumpp_nonce_bug() {
	{
		pfp_warn("test");

				std::string nonce_used_raw(24,0);
				nonce_used_raw.at(23)=6;

				pfp_dbg1("nonce_used_raw="<<to_debug(nonce_used_raw));
				antinet_crypto::t_crypto_nonce nonce_used(
					sodiumpp::encoded_bytes(nonce_used_raw , sodiumpp::encoding::binary)
				);
				auto x = nonce_used;
				pfp_warn("copy ok");
				auto y = nonce_used.get();
				pfp_warn("get ok");

				pfp_warn("Received NONCE=" << antinet_crypto::show_nice_nonce(nonce_used) );

				pfp_warn("OK?");
				return false;
	}
}

bool wip_galaxy_route_doublestar(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
//	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;
        pfp_fact( mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr );

	// --- define the test world ---
	// for given peer-number - the properties of said peer as seen by us (pubkey, ip - things given on the command line):
	std::map< int , std::string > peer_cmd_map = {
		{ 1 , "192.168.1.62:9042-fd42:9fd1:ce03:9edf:1d7e:2257:b651:d89f" } ,
		{ 2 , "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b" } ,
		{ 3 , "192.168.3.62:9042-fd42:5516:34c7:9302:890f:0a2d:5586:79ee" } ,
	};

	// list of connections in our test-world:
	std::map< int , std::vector<int> > peer_to_peer; // for given peer that we will create: list of his peer-number(s) that he peers into
	peer_to_peer[1] = std::vector<int>{ 2 , 3 };
	/*
	peer_to_peer[2] = vector<int>{ 4 , 5 };
	peer_to_peer[3] = vector<int>{ 6 , 7 };
	peer_to_peer[4] = vector<int>{ };
	peer_to_peer[5] = vector<int>{ };
	peer_to_peer[6] = vector<int>{ };
	peer_to_peer[7] = vector<int>{ };
	*/

	for (int peer_nr : peer_to_peer.at(my_nr)) { // for me, add the --peer refrence of all peers that I should peer into:
//		pfp_info("I connect into demo peer number: " << peer_nr);
                pfp_info(mo_file_reader::gettext("L_connect_into_demo_peer_nr") << peer_nr);

		add_program_option_vector_strings(argm, "peer", peer_cmd_map.at(peer_nr));
	}

//	pfp_warn("Remember to set proper HOME with your key pair!");
        pfp_warn(mo_file_reader::gettext("L_remember_set_proper_home"));

	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(my_nr) , false );

/* TODO(r) bug#m153
	boost::any boostany = argm.at("peer");
	pfp_erro("PEER = " << boostany);
	try { auto vvv = boost::any_cast<vector<string>>( argm.at("peer") ); }
	catch(...) { pfp_warn("EXCEPT"); }
	pfp_warn("after");
*/
	// pfp_note("Done dev setup, runnig as: " << to_debug(argm));
	return true;
}

} // namespace developer_tests

// ============================================================================


/**
 * @brief Loads name of demo from demo.conf.
 * @TODO just loads file from current PWD, should instead load from program's starting pwd and also search user home dir.
 * @return Name of the demo to run, as configured in demo.conf -or- string "default" if can not load it.
*/
std::string demoname_load_conf(std::string democonf_fn = "config/demo.conf") {
	std::string ret="default";
	try {
		std::ifstream democonf_file(democonf_fn);
//		if (! democonf_file.good()) { std::cerr<<"Not loading demo user config file ("<<democonf_fn<<")" << std::endl; return ret; }
                if (! democonf_file.good()) { pfp_fact( mo_file_reader::gettext("L_not_load_demo_usr_conf_file")<<democonf_fn<<")" ); return ret; }

		std::string line="";
		getline(democonf_file,line);
//		if (! democonf_file.good()) { std::cerr<<"Failure in parsing demo user config file ("<<democonf_fn<<")" << std::endl; return ret; }
                if (! democonf_file.good()) { pfp_fact( mo_file_reader::gettext("L_faliture_parsing_demo_usr_conf_file")<<democonf_fn<<")" ); return ret; }

		ret = line.substr( std::string("demo=").size() );
	} catch(...) { }
//	std::cerr<<"Loaded demo user config file ("<<democonf_fn<<") with demo option:" << ret << std::endl;
        pfp_fact( mo_file_reader::gettext("L_loaded_demo_usr_conf_file") <<democonf_fn << mo_file_reader::gettext("L_with_demo_options") << ret );

	return ret;
}

bool test_foo() {
	pfp_info("TEST FOO");
	return false;
}

bool test_bar() {
	pfp_info("TEST BAR");
	return false;
}

void test_lang_optional() {
	/* this fails on boost from debian 7
	boost::optional<float> x = boost::none;
	if (x) pfp_info("too early: " << *x );
	x = 3.14;
	if (x) pfp_info("and now: " << x.value() );
	*/
}

/**
@brief Run the main developer test in this code version (e.g. on this code branch / git branch)
@param argm - map with program options, it CAN BE MODIFIED here, e.g. the test can be to set some options and let the program continue
@return false if the program should quit after this test
*/
bool run_mode_developer_main(boost::program_options::variables_map & argm) {
//	std::cerr << "Running in developer/demo mode." << std::endl;
        pfp_fact( mo_file_reader::gettext("L_devel_demo_running_mode") << std::endl );

	const std::string demoname_default = g_demoname_default;
	auto demoname = argm["develdemo"].as<std::string>();
	pfp_note("Demoname (from program options command line) is:" << demoname);

	namespace po = boost::program_options;
	po::options_description desc("Possible demos");
	desc.add_options()
					// ("none", "no demo: start program in normal mode instead (e.g. to ignore demo config file)")
					("lang_optional",			"foo boost::optional<>")
					("sodiumpp_bug",			"sodiumpp nonce overflow constructor bug test (hits on older sodiumpp version)")
					("foo",						"foo test")
					("bar",						"bar test")
					("serialize",				"serialize test")
					("crypto",					"crypto test")
					("gen_key_bench",			"crypto benchmark")
					("crypto_stream_bench",		"crypto stream benchmark")
					("ct_bench",				"crypto tunel benchmark")
					("route_dij",				"dijkstra test")
					("route",					"current best routing (could be equal to some other test)")
					("debug",					"some of the debug/logging functions")
					("rpc",						"rpc demo")
					("help",					"Help msg");

	if ((demoname=="help")||(demoname=="list")) {
		pfp_fact( "\nAvailable options for --demo NAME (or --devel --develdemo NAME) are following:" );
		pfp_fact( desc << "\nChoose one of them as the NAME. But type it without the leading -- [TODO]" ); // TODO(janusz)
		return false;
	}

	const std::string demoname_loaded = demoname_load_conf();
	if (demoname_loaded != "default") demoname = demoname_loaded;
	if (demoname=="hardcoded") demoname = demoname_default;

	pfp_note("Demo name selected: [" << demoname << "]");
	pfp_fact( std::string(70,'=')<<"\n" << "Demo: " << demoname << std::endl
		<< std::string(70,'=')<<"\n" );

	if (demoname=="lang_optional") { test_lang_optional();  return false; }
	if (demoname=="foo") { test_foo();  return false; }
	if (demoname=="bar") { test_bar();  return false; }
	if (demoname=="crypto") { antinet_crypto::test_crypto();  return false; }
	if (demoname=="gen_key_bench") { antinet_crypto::generate_keypairs_benchmark(2);  return false; }
	if (demoname=="crypto_stream_bench") { antinet_crypto::stream_encrypt_benchmark(2); return false; }
	if (demoname=="ct_bench") { antinet_crypto::multi_key_sign_generation_benchmark(2); return false; }
	if (demoname=="route_dij") { return developer_tests::wip_galaxy_route_doublestar(argm); }
	if (demoname=="route"    ) { return developer_tests::wip_galaxy_route_doublestar(argm); }
	//if (demoname=="rpc") { rpc_demo(); return false; }
	if (demoname=="debug") { unittest::test_debug1(); return false; }

	pfp_warn("Unknown Demo option ["<<demoname<<"] try giving other name, e.g. run program with --develdemo");
	return false;
}

bool run_mode_developer(boost::program_options::variables_map & argm) {
	auto ret = run_mode_developer_main(argm);
	pfp_fact( std::string(70,'=')<<"\n" );
	return ret;
}

// ============================================================================
// ============================================================================
// ============================================================================

#define _early_cerr( X ) do { std::cerr << X << std::endl; } while(0)

/**
 * Purpose of this function is to print some compilation "flavour" flags, e.g. is this a TSAN build or not.
 * @warning This function must be called from main(), very early (before other output or setting i/o)
 * and the caller should exit immediatelly after,
 * because this function uses normal conventions (e.g. of using std::cout) instead following ones from program/project
 * (e.g. it does not use program/project debug/console tools).
 */
void main_print_flavour() {
	auto & out = std::cout; // we can directly use std::cout, in this special function
	out << "# Flavour:" << std::endl;
	bool valgrind_memory_is_possible = true; // do conditions allow us to run in valgrind, e.g. TSAN maps too much memory so then =false
	#if FLAVOUR_TSAN_FULL
		valgrind_memory_is_possible = false;
		out << "FLAVOUR_TSAN_FULL" << std::endl;
	#endif
	#if FLAVOUR_UBSAN_FULL
		out << "FLAVOUR_UBSAN_FULL" << std::endl;
	#endif
	#if FLAVOUR_UBSAN_ONLY_REPORTS
		out << "FLAVOUR_UBSAN_ONLY_REPORTS" << std::endl;
	#endif

	out <<  ( valgrind_memory_is_possible ? "valgrind_memory_is_possible" : "(valgrind NOT possible)" ) << std::endl;
}




int main(int argc, const char * const * argv) { // the main() function
	// parse early options:
	// this is done very early, we do not use console, nor boost program_options etc
	std::string argt_exec = (argc>=1) ? argv[0] : ""; // exec name
	std::vector<std::string> argt; // args (without exec name)
	for (int i=1; i<argc; ++i) argt.push_back(argv[i]);
	bool early_debug = contains_value(argt, "--d");

	if (contains_value(argt,"--print-flags-flavour")) {
		main_print_flavour();
		return 0; // <--- exit
	}


	try{
		auto *result = std::setlocale(LC_ALL, "en_US.UTF-8");
		if (result == nullptr) throw std::runtime_error("Can not set locale (first)");
	}catch (...){
		std::cerr<<"Error: setlocale."<<std::endl;
	}

	// This code MUST be 1-thread and very early in main
	#ifdef ENABLE_LIB_CURL
	CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
	if(res != CURLE_OK) {
		bitcoin_node_cli::curl_initialized=false;
		std::cerr<<"Error: lib curl init."<<std::endl;
	}
	else{
		bitcoin_node_cli::curl_initialized=true;
	}
	#else
		std::cerr<<"Warning: lib curl is disabled."<<std::endl;
	#endif

	enum class t_program_type {
		e_program_type_tunserver = 1,
		e_program_type_newloop = 100,
	};

	const t_program_type program_type = [&argt] {
		if (remove_and_count(argt, "--newloop" ))
			return t_program_type::e_program_type_newloop;
		else
			return t_program_type::e_program_type_tunserver;
	}();

	std::unique_ptr<c_the_program> the_program = nullptr;
	switch (program_type) {
		case t_program_type::e_program_type_tunserver:
			the_program = std::make_unique<c_the_program_tunserver>();
		break;
		case t_program_type::e_program_type_newloop:
			the_program = std::make_unique<c_the_program_newloop>();
		break;
		default: break;
	}

	if (!the_program) {
		_early_cerr("Programming error: not supported program type.");
		return 1;
	}

	the_program->take_args(argt_exec , argt); // takes again args, with removed special early args
	the_program->startup_console_first();
	the_program->startup_locales_early();
	the_program->startup_data_dir();
	the_program->startup_curl();
	the_program->startup_version();
	the_program->startup_locales_later();
	the_program->init_library_sodium();

	g_dbg_level = 60;
	if (early_debug) g_dbg_level_set(20, mo_file_reader::gettext("L_early_debug_comand_line"));

	if ( remove_and_count(argt,"--mode-bench") ) {
		c_string_string_Cstyle args_cstyle( argt_exec , argt );
		const int again_argc = args_cstyle.get_argc();
		const char ** again_argv = args_cstyle.get_argv();
		return n_netmodel::netmodel_main( again_argc , again_argv );
	}

	my_cap::drop_root(remove_and_count(argt, "--home-env" )); // [SECURITY] if we are started as root, then here drop the UID/GID (we retain CAPs).

	my_cap::drop_privileges_on_startup(); // [SECURITY] drop unneeded privileges (more will be dropped later)
	// we drop privilages here, quite soon on startup. Not before, because we choose to have configured console
	// to report any problems with CAPs (eg compatibility issues),
	// and we expect to have no exploitable code in this short code to setup console and show version
	// (especially as none of it depends on user provided inputs)

	{
		bool done; int ret; std::tie(done,ret) = the_program->program_startup_special();
		if (done) return ret;
	}

	the_program->options_create_desc();

	the_program->options_parse_first();
	the_program->options_multioptions();
	the_program->options_done();

	{
		bool done; int ret; std::tie(done,ret) = the_program->options_commands_run();
		if (done) return ret;
	}

	int exit_code=1;
	bool exception_catched = true;
	try {

		exit_code = the_program->main_execution(); // <---
		exception_catched = false;

	} // try running server
	catch(const ui::exception_error_exit &) {
		pfp_erro( mo_file_reader::gettext("L_exiting_explained_above") );
		exit_code = 1;
	}
	catch(const capmodpp::capmodpp_error & e) {
		pfp_erro( mo_file_reader::gettext("L_unhandled_exception_running_server") << ' '
			<< "(capmodpp_error) "
			<< e.what() << mo_file_reader::gettext("L_exit_aplication") );
		exit_code = 2;
	}
	catch(const std::exception& e) {
		pfp_erro( mo_file_reader::gettext("L_unhandled_exception_running_server") << ' '
			<< e.what() << mo_file_reader::gettext("L_exit_aplication") );
		exit_code = 2;
	}
	catch(...) {
		pfp_erro( mo_file_reader::gettext("L_unknown_exception_running_server") );
		exit_code = 3;
	}

	try {
		if( !exception_catched )
			pfp_note(mo_file_reader::gettext("L_exit_no_error"));
	}
	catch(...) {
		std::cerr<<"(Error in printing previous error)";
	}

	#ifdef ENABLE_LIB_CURL
		curl_global_cleanup();
	#endif
	bitcoin_node_cli::curl_initialized=false;

	return exit_code;
}

