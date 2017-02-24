// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "the_program.hpp"
#include "the_program_tunserver.hpp"

namespace developer_tests {

string make_pubkey_for_peer_nr(int peer_nr) {
	string peer_pub = string("4a4b4c") + string("4") + string(1, char('0' + peer_nr)  );
	return peer_pub;
}

// list of peers that exist in our test-world:
struct t_peer_cmdline_ref {
	string ip;
	string pubkey;
	string privkey; ///< just in the test world. here we have knowledge of peer's secret key
};

bool wip_galaxy_route_star(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int node_nr = argm["develnum"].as<int>();  assert( (node_nr>=1) && (node_nr<=254) );
        _fact( mo_file_reader::gettext("L_running_devel_as_node_nr") << node_nr );

	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");

	int peer_nr = node_nr==1 ? 2 : 1;

	string peer_pub = make_pubkey_for_peer_nr( peer_nr );
	// each connect to node .1., except the node 1 that connects to .2."
	string peer_ip = string("192.168.") + std::to_string( peer_nr  ) + string(".62");

//	_mark("Developer: adding peer with arguments: ip=" << peer_ip << " pub=" << peer_pub );
        _mark(mo_file_reader::gettext("L_devel_adding_peer_with_args") << "ip=" << peer_ip << " pub=" << peer_pub );

	// argm.insert(std::make_pair("K", po::variable_value( int(node_nr) , false )));
	argm.insert(std::make_pair("peerip", po::variable_value( peer_ip , false )));
	argm.at("peerpub") = po::variable_value( peer_pub , false );
	argm.at("mypub") = po::variable_value( make_pubkey_for_peer_nr(node_nr)  , false );
	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(node_nr) , false );
	return true; // continue the test
}

void add_program_option_vector_strings(boost::program_options::variables_map & argm,
	const string & name , const string & value_to_append)
{
	namespace po = boost::program_options;
	vector<string> old_peer;
	try {
		old_peer = argm[name].as<vector<string>>();
		old_peer.push_back(value_to_append);
		argm.at(name) = po::variable_value( old_peer , false );
	} catch(boost::bad_any_cast) {
		old_peer.push_back(value_to_append);
		argm.insert( std::make_pair(name , po::variable_value( old_peer , false )) );
	}
//	_info("program options: added to option '" << name << "' - now size: " << argm.at(name).as<vector<string>>().size() );
        _info(mo_file_reader::gettext("L_program_option_added") << name << mo_file_reader::gettext("L_option_now_size") << argm.at(name).as<vector<string>>().size() );

}

bool wip_galaxy_route_pair(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
//	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;
        _fact( mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr );

	if (my_nr == 1) add_program_option_vector_strings(argm, "peer", "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b");
	if (my_nr == 2) add_program_option_vector_strings(argm, "peer", "192.168.1.62:9042-fd42:ae11:f636:8636:ae76:acf5:e5c4:dae1");

	return true;
}

bool demo_sodiumpp_nonce_bug() {
	{
		_warn("test");

				string nonce_used_raw(24,0);
				nonce_used_raw.at(23)=6;

				_dbg1("nonce_used_raw="<<to_debug(nonce_used_raw));
				antinet_crypto::t_crypto_nonce nonce_used(
					sodiumpp::encoded_bytes(nonce_used_raw , sodiumpp::encoding::binary)
				);
				auto x = nonce_used;
				_warn("copy ok");
				auto y = nonce_used.get();
				_warn("get ok");

				_warn("Received NONCE=" << antinet_crypto::show_nice_nonce(nonce_used) );

				_warn("OK?");
				return false;
	}
}

bool wip_galaxy_route_doublestar(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
//	std::cerr << "Running in developer mode - as my_nr=" << my_nr << std::endl;
        _fact( mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr );

	// --- define the test world ---
	// for given peer-number - the properties of said peer as seen by us (pubkey, ip - things given on the command line):
	map< int , std::string > peer_cmd_map = {
		{ 1 , "192.168.1.62:9042-fd42:9fd1:ce03:9edf:1d7e:2257:b651:d89f" } ,
		{ 2 , "192.168.2.62:9042-fd42:10a9:4318:509b:80ab:8042:6275:609b" } ,
		{ 3 , "192.168.3.62:9042-fd42:5516:34c7:9302:890f:0a2d:5586:79ee" } ,
	};

	// list of connections in our test-world:
	map< int , vector<int> > peer_to_peer; // for given peer that we will create: list of his peer-number(s) that he peers into
	peer_to_peer[1] = vector<int>{ 2 , 3 };
	/*
	peer_to_peer[2] = vector<int>{ 4 , 5 };
	peer_to_peer[3] = vector<int>{ 6 , 7 };
	peer_to_peer[4] = vector<int>{ };
	peer_to_peer[5] = vector<int>{ };
	peer_to_peer[6] = vector<int>{ };
	peer_to_peer[7] = vector<int>{ };
	*/

	for (int peer_nr : peer_to_peer.at(my_nr)) { // for me, add the --peer refrence of all peers that I should peer into:
//		_info("I connect into demo peer number: " << peer_nr);
                _info(mo_file_reader::gettext("L_connect_into_demo_peer_nr") << peer_nr);

		add_program_option_vector_strings(argm, "peer", peer_cmd_map.at(peer_nr));
	}

//	_warn("Remember to set proper HOME with your key pair!");
        _warn(mo_file_reader::gettext("L_remember_set_proper_home"));

	argm.at("myname") = po::variable_value( "testnode-" + std::to_string(my_nr) , false );

/* TODO(r) bug#m153
	boost::any boostany = argm.at("peer");
	_erro("PEER = " << boostany);
	try { auto vvv = boost::any_cast<vector<string>>( argm.at("peer") ); }
	catch(...) { _warn("EXCEPT"); }
	_warn("after");
*/
	// _note("Done dev setup, runnig as: " << to_debug(argm));
	return true;
}

} // namespace developer_tests

// ============================================================================


/***
 * @brief Loads name of demo from demo.conf.
 * @TODO just loads file from current PWD, should instead load from program's starting pwd and also search user home dir.
 * @return Name of the demo to run, as configured in demo.conf -or- string "default" if can not load it.
*/
string demoname_load_conf(std::string democonf_fn = "config/demo.conf") {
	string ret="default";
	try {
		ifstream democonf_file(democonf_fn);
//		if (! democonf_file.good()) { std::cerr<<"Not loading demo user config file ("<<democonf_fn<<")" << std::endl; return ret; }
                if (! democonf_file.good()) { _fact( mo_file_reader::gettext("L_not_load_demo_usr_conf_file")<<democonf_fn<<")" ); return ret; }

		string line="";
		getline(democonf_file,line);
//		if (! democonf_file.good()) { std::cerr<<"Failure in parsing demo user config file ("<<democonf_fn<<")" << std::endl; return ret; }
                if (! democonf_file.good()) { _fact( mo_file_reader::gettext("L_faliture_parsing_demo_usr_conf_file")<<democonf_fn<<")" ); return ret; }

		ret = line.substr( string("demo=").size() );
	} catch(...) { }
//	std::cerr<<"Loaded demo user config file ("<<democonf_fn<<") with demo option:" << ret << std::endl;
        _fact( mo_file_reader::gettext("L_loaded_demo_usr_conf_file") <<democonf_fn << mo_file_reader::gettext("L_with_demo_options") << ret );

	return ret;
}

bool test_foo() {
	_info("TEST FOO");
	return false;
}

bool test_bar() {
	_info("TEST BAR");
	return false;
}

void test_lang_optional() {
	/* this fails on boost from debian 7
	boost::optional<float> x = boost::none;
	if (x) _info("too early: " << *x );
	x = 3.14;
	if (x) _info("and now: " << x.value() );
	*/
}

/***
@brief Run the main developer test in this code version (e.g. on this code branch / git branch)
@param argm - map with program options, it CAN BE MODIFIED here, e.g. the test can be to set some options and let the program continue
@return false if the program should quit after this test
*/
bool run_mode_developer_main(boost::program_options::variables_map & argm) {
//	std::cerr << "Running in developer/demo mode." << std::endl;
        _fact( mo_file_reader::gettext("L_devel_demo_running_mode") << std::endl );

	const string demoname_default = g_demoname_default;
	auto demoname = argm["develdemo"].as<string>();
	_note("Demoname (from program options command line) is:" << demoname);

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
		_fact( "\nAvailable options for --demo NAME (or --devel --develdemo NAME) are following:" );
		_fact( desc << "\nChoose one of them as the NAME. But type it without the leading -- [TODO]" ); // TODO(janusz)
		return false;
	}

	const string demoname_loaded = demoname_load_conf();
	if (demoname_loaded != "default") demoname = demoname_loaded;
	if (demoname=="hardcoded") demoname = demoname_default;

	_note("Demo name selected: [" << demoname << "]");
	_fact( std::string(70,'=')<<"\n" << "Demo: " << demoname << endl
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

	_warn("Unknown Demo option ["<<demoname<<"] try giving other name, e.g. run program with --develdemo");
	return false;
}

bool run_mode_developer(boost::program_options::variables_map & argm) {
	auto ret = run_mode_developer_main(argm);
	_fact( std::string(70,'=')<<"\n" );
	return ret;
}


// ============================================================================
// ============================================================================
// ============================================================================

int main(int argc, const char **argv) {
	unique_ptr<c_the_program> the_program = make_unique<c_the_program_tunserver>();

	the_program->take_args(argc,argv);
	the_program->startup_console_first();
	the_program->startup_version();
	the_program->startup_data_dir();
	{
		bool done; int ret; std::tie(done,ret) = the_program->program_startup_special();
		if (done) return ret;
	}
	the_program->startup_locales();

	g_dbg_level = 60;
	bool early_debug=false;
	for (decltype(argc) i=0; i<argc; ++i) if (  (!strcmp(argv[i],"--d")) || (!strcmp(argv[i],"--debug"))  ) early_debug=true;
	if (early_debug) g_dbg_level_set(20, mo_file_reader::gettext("L_early_debug_comand_line"));

	the_program->init_library_sodium();

	the_program->options_create_desc();

	the_program->options_parse_first();
	the_program->options_multioptions();
	the_program->options_done();

	{
		bool done; int ret; std::tie(done,ret) = the_program->options_commands_run();
		if (done) return ret;
	}

	int exit_code=1;
	try {

		exit_code = the_program->main_execution(); // <---

	} // try running server
	catch(ui::exception_error_exit) {
		_erro( mo_file_reader::gettext("L_exiting_explained_above") );
		return 1;
	}
	catch(std::exception& e) {
		_erro( mo_file_reader::gettext("L_unhandled_exception_running_server") << ' '
		 << e.what() << mo_file_reader::gettext("L_exit_aplication") );
		return 2;
	}
	catch(...) {
		_erro( mo_file_reader::gettext("L_unknown_exception_running_server") );
		return 3;
	}
	_note(mo_file_reader::gettext("L_exit_no_error")); return 0;

  return exit_code;
}

