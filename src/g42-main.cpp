// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#include "tunserver.hpp"

#include "project.hpp"

#include "glue_sodiumpp_crypto.hpp"

#include "ui.hpp"
#include "trivialserialize.hpp"
#include "datastore.hpp"

#include "rpc/rpc.hpp"
#include "galaxy_debug.hpp"

#include "c_json_genconf.hpp"
#include "c_json_load.hpp"

#include <mo_reader.hpp>

#ifdef HTTP_DBG
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include "httpdbg/httpdbg-server.hpp"
#endif

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
        std::cerr << mo_file_reader::gettext("L_running_devel_as_node_nr") << node_nr << std::endl;

	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");

	int peer_nr = node_nr==1 ? 2 : 1;

	string peer_pub = make_pubkey_for_peer_nr( peer_nr );
	// each connect to node .1., except the node 1 that connects to .2."
	string peer_ip = string("192.168.") + std::to_string( peer_nr  ) + string(".62");

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
    _info(mo_file_reader::gettext("L_program_option_added") << name << mo_file_reader::gettext("L_option_now_size") << argm.at(name).as<vector<string>>().size() );

}

bool wip_galaxy_route_pair(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	const int my_nr = argm["develnum"].as<int>();  assert( (my_nr>=1) && (my_nr<=254) ); // number of my node
    std::cerr << mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr << std::endl;

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
    std::cerr << mo_file_reader::gettext("L_devel_mode_as_nr") << my_nr << std::endl;

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
		_info(mo_file_reader::gettext("L_connect_into_demo_peer_nr") << peer_nr);

		add_program_option_vector_strings(argm, "peer", peer_cmd_map.at(peer_nr));
	}

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

/***
 * @brief Loads name of demo from demo.conf.
 * @TODO just loads file from current PWD, should instead load from program's starting pwd and also search user home dir.
 * @return Name of the demo to run, as configured in demo.conf -or- string "default" if can not load it.
*/
string demoname_load_conf(std::string democonf_fn = "config/demo.conf") {
	string ret="default";
	try {
		ifstream democonf_file(democonf_fn);
			if (! democonf_file.good()) { std::cerr<<mo_file_reader::gettext("L_not_load_demo_usr_conf_file")<<democonf_fn<<")" << std::endl; return ret; }

		string line="";
		getline(democonf_file,line);
            if (! democonf_file.good()) { std::cerr<<mo_file_reader::gettext("L_faliture_parsing_demo_usr_conf_file")<<democonf_fn<<")" << std::endl; return ret; }

		ret = line.substr( string("demo=").size() );
	} catch(...) { }
    std::cerr<< mo_file_reader::gettext("L_loaded_demo_usr_conf_file") <<democonf_fn << mo_file_reader::gettext("L_with_demo_options") << ret << std::endl;

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
    std::cerr << mo_file_reader::gettext("L_devel_demo_running_mode") << std::endl;

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
		std::cout << "\nAvailable options for --demo NAME (or --devel --develdemo NAME) are following:";
		std::cout << desc << "\nChoose one of them as the NAME. But type it without the leading -- [TODO]" << std::endl; // TODO(janusz)
		return false;
	}

	const string demoname_loaded = demoname_load_conf();
	if (demoname_loaded != "default") demoname = demoname_loaded;
	if (demoname=="hardcoded") demoname = demoname_default;

	_note("Demo name selected: [" << demoname << "]");
	std::cout << std::string(70,'=')<<"\n" << "Demo: " << demoname << endl
		<< std::string(70,'=')<<"\n" << std::endl;

	if (demoname=="lang_optional") { test_lang_optional();  return false; }
	if (demoname=="foo") { test_foo();  return false; }
	if (demoname=="bar") { test_bar();  return false; }
	if (demoname=="serialize") { trivialserialize::test::test_trivialserialize(std::cout);  return false; }
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
	std::cout << std::string(70,'=')<<"\n" << std::endl;
	return ret;
}

int main(int argc, char **argv) {
//	std::cerr << std::string(80,'=') << std::endl << g_the_disclaimer << std::endl << std::endl;

	using std::cerr; using std::endl;

	cerr << "Start... " << endl;
	string install_dir_base; // here we will find main dir like "/usr/" that contains our share dir

	{
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
			selfpath += argv[0];
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
				cerr << "Test: [" << testname << "]... " << std::flush;
				ifstream filetest( testname.c_str() );
				if (filetest.good()) {
					install_dir_base = dir;
					found=true;
					cerr << " OK! " << endl;
					break;
				} else cerr << endl;
			}
		} catch(std::exception & ex) {
				cerr << "Error while looking for data directory ("<<ex.what()<<")" << std::endl;
		}
		if (found) {
		} else {
			cerr << "Can not find language data files." << endl;
		}
	}

	std::cerr << "Data: [" << install_dir_base << "]" << endl;
	const std::string install_dir_share_locale = install_dir_base + "/share/locale";
	std::cerr << "Lang: [" << install_dir_share_locale << "]" << endl;
	setlocale(LC_ALL,"");

/*	boost::locale::generator gen;
	// Specify location of dictionaries
	gen.add_messages_path(install_dir_share_locale);
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
		mo_reader.add_messages_dir(install_dir_share_locale);
		mo_reader.add_mo_filename("galaxy42_main");
		mo_reader.read_file();
	} catch (const std::exception &e) {
		std::cerr << "mo file open error: " << e.what() << std::endl;
	}

	std::cerr << std::string(80,'=') << std::endl << mo_file_reader::gettext("L_warning_work_in_progres") << std::endl << std::endl;
	std::cerr << mo_file_reader::gettext("L_program_is_pre_pre_alpha") << std::endl;
	std::cerr << mo_file_reader::gettext("L_program_is_copyrighted") << std::endl;
	std::cerr << std::endl;


//	const std::string install_dir_share_locale="share/locale"; // for now, for running in place
//	setlocale(LC_ALL,"");
//	string used_domain = bindtextdomain ("galaxy42_main", install_dir_share_locale.c_str() );
//	textdomain("galaxy42_main");
	// Using mo_file_reader::gettext:
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_pre_pre_alpha") << std::endl;
//	std::cerr << mo_reader::mo_file_reader::gettext("L_program_is_copyrighted") << std::endl;

	const int config_default_basic_dbg_level = 60; // [debug] level default
	const int config_default_incrased_dbg_level = 20; // [debug] early-debug level if user used --d

	g_dbg_level = config_default_basic_dbg_level;
    bool early_debug=false;

//	_fact("Using data dir ["<<install_dir_base<<"], language/locale ["<<locale_name<<"]");

	// === data-dir, languages, locale, debug - are now set ===

    #ifdef HTTP_DBG
    int http_dbg_port = 9080;
    #endif
	for (decltype(argc) i=0; i<argc; ++i) if (  (!strcmp(argv[i],"--d")) || (!strcmp(argv[i],"--debug"))  ) early_debug=true;
    if (early_debug) g_dbg_level_set(config_default_incrased_dbg_level, mo_file_reader::gettext("L_early_debug_comand_line"));

	{
            _info(mo_file_reader::gettext("L_starting_lib_libsodium"));

		if (sodium_init() == -1) {
                    _throw_error( std::runtime_error(mo_file_reader::gettext("L_lisodium_init_err")) );

		}
				_info(mo_file_reader::gettext("L_libsodium_ready"));

	}
	try {
		std::unique_ptr<c_tunserver> myserver_ptr;
		namespace po = boost::program_options;
		unsigned line_length = 120;

		const string config_default_myname = "galaxy";

		auto desc = make_shared<po::options_description>( mo_file_reader::gettext("L_options") , line_length);
		desc->add_options()
                    ("help", mo_file_reader::gettext("L_what_help_do").c_str())

                    ("h", mo_file_reader::gettext("L_what_h_do").c_str())

                    ("debug", mo_file_reader::gettext("L_what_debug_do").c_str())

                    ("d", mo_file_reader::gettext("L_what_d_do").c_str())

                    ("quiet", mo_file_reader::gettext("L_what_quiet_do").c_str())

                    ("q", mo_file_reader::gettext("L_what_q_do").c_str())

                    ("peer", po::value<std::vector<std::string>>()->multitoken(), mo_file_reader::gettext("L_what_peer_do").c_str())

                    ("info", mo_file_reader::gettext("L_what_info_do").c_str())

                    ("list-my-keys", mo_file_reader::gettext("L_what_listMyKeys_do").c_str())

                    ("my-key", po::value<std::string>(), mo_file_reader::gettext("L_what_myKey_do").c_str())

                    ("my-key-file", po::value<std::string>(), mo_file_reader::gettext("L_what_myKeyFile_do").c_str())

                    ("config", po::value<std::string>()->default_value("galaxy.conf") , mo_file_reader::gettext("L_what_config_do").c_str())

                    ("no-config", mo_file_reader::gettext("L_what_noConfig_do").c_str())

                    ("gen-key-simple", mo_file_reader::gettext("L_what_genKeySimple_do").c_str())
            #ifdef HTTP_DBG
                        ("http-dbg-port", po::value<int>()->default_value(9080), mo_file_reader::gettext("L_what_httpDbgPort_do").c_str())
            #endif
                        ("net-hello-interval", po::value<int>()->default_value(3), mo_file_reader::gettext("L_what_netHelloInterval_do").c_str())
                        ("port", po::value<int>()->default_value(9042), mo_file_reader::gettext("L_port_do").c_str())
                        ("rpc-port", po::value<int>()->default_value(9043), mo_file_reader::gettext("L_rpcPort_do").c_str())

			#if EXTLEVEL_IS_PREVIEW
/*
//			("gen-key", "COMMAND: Generate combination of crypto key"
                        ("gen-key", mo_reader::mo_file_reader::gettext("L_gen_combo_crypto_key_require_examples").c_str())

//						"\nrequired [--new-key or --new-key-file, --key-type]\nexamples:"
//                                                mo_reader::mo_file_reader::gettext("L_genKey_require_examples");
//						"\n--gen-key --new-key \"myself\" --key-type \"ed25519:x3\" \"rsa:x1:size=4096\""
//						"\n--gen-key --new-key-file \"~/Documents/work/newkey.PRV\""
//						"--key-type \"ed25519:x3\" \"rsa:x1:size=4096\"")
//				("new-key", po::value<std::string>(), "Name of output key file in default location for keys")
                                ("new-key", po::value<std::string>(), mo_reader::mo_file_reader::gettext("L_what_newKey_do").c_str())

//				("new-key-file", po::value<std::string>(), "Name of output key file in specified location")
                                ("new-key-file", po::value<std::string>(), mo_reader::mo_file_reader::gettext("L_what_newKeyFile_do").c_str())

//				("key-type", po::value<std::vector<std::string>>()->multitoken(), "Types of generated sub keys")
                                ("key-type", po::value<std::vector<std::string>>()->multitoken(), mo_reader::mo_file_reader::gettext("L_what_keyType_do").c_str())
*/
			#endif

			#if EXTLEVEL_IS_PREVIEW


			("demo", po::value<std::string>()->default_value(""), mo_file_reader::gettext("L_what_demo_do").c_str())

			("devel",mo_file_reader::gettext("L_what_devel_do").c_str())

			("develnum", po::value<int>()->default_value(1), mo_file_reader::gettext("L_what_decelnum_do").c_str())

			("develdemo", po::value<std::string>()->default_value("hardcoded"), mo_file_reader::gettext("L_what_develdemo_do").c_str())

			// ("K", po::value<int>()->required(), "number that sets your virtual IP address for now, 0-255")
			("myname", po::value<std::string>()->default_value(config_default_myname) , mo_file_reader::gettext("L_what_myname_do").c_str())

                        ("gen-config", mo_file_reader::gettext("L_what_gen_config_do").c_str())

                        ("set-IDI", mo_file_reader::gettext("L_what_set_IDI_do").c_str())

                        ("sign",  mo_file_reader::gettext("L_what_sing_do").c_str())

                        ("sign-key", po::value<std::string>(), mo_file_reader::gettext("L_what_singKey_do").c_str())

                        ("sign-key-file", po::value<std::string>(), mo_file_reader::gettext("L_what_singKeyFile_do").c_str())

                        ("sign-data-file", po::value<std::string>(), mo_file_reader::gettext("L_what_singDataFile_do").c_str())

                        ("verify", mo_file_reader::gettext("L_what_verify_do").c_str())

                        ("trusted-key", po::value<std::string>(), mo_file_reader::gettext("L_what_trustedKey_do").c_str())

                        ("trusted-key-file", po::value<std::string>(), mo_file_reader::gettext("L_what_trustedKeyFile_do").c_str())

                        ("toverify-key", po::value<std::string>(), mo_file_reader::gettext("L_what_toverifyKey_do").c_str())

                        ("toverify-key-file", po::value<std::string>(), mo_file_reader::gettext("L_what_toverifyKeyFile_do").c_str())

                        ("toverify-data-file", po::value<std::string>(), mo_file_reader::gettext("L_what_toverifyDataFile_do").c_str())

			/*
				("signature-file", po::value<std::string>(),
//							"External Name of signature file in specified location"
//							"\nDefault signature file name = key/data file name + \".sig\" extension")
                                                        mo_file_reader::gettext("L_what_signatureFile_do").c_str())
				*/


			#endif

			;


        _note(mo_file_reader::gettext("L_parse_program_option"));

		po::variables_map argm;
		try { // try parsing
			po::store(po::parse_command_line(argc, argv, *desc), argm); // <-- parse actuall real command line options
//			_note("BoostPO parsed argm size=" << argm.size());
                        _note("BoostPO parsed argm size=" << argm.size());

			// === PECIAL options - that set up other program options ===

			#if EXTLEVEL_IS_PREVIEW
//			_info("BoostPO Will parse demo/devel options");
                        _info("BoostPO Will parse demo/devel options");

			{ // Convert shortcut options:  "--demo foo"   ----->   "--devel --develdemo foo"
				auto opt_demo = argm["demo"].as<string>();
				if ( opt_demo!="" ) {
//					g_dbg_level_set(10,"Running in demo mode");
                                        g_dbg_level_set(10,"Running in demo mode");

//					_info("The demo command line option is given:" << opt_demo);
                                        _info("The demo command line option is given:" << opt_demo);

					// argm.insert(std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.at("develdemo") = po::variable_value( opt_demo , false );
					// (std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.insert(std::make_pair("devel",     po::variable_value( false , false ))); // --devel
				}
			}

			if (argm.count("devel")) { // can also set up additional options
				try {
					g_dbg_level_set(10,"Running in devel mode");
					_info("The devel mode is active");

					bool should_continue = run_mode_developer(argm);
					if (!should_continue) return 0;
				}
				catch(std::exception& e) {
                    std::cerr << mo_file_reader::gettext("L_unhandled_exception_devel_mode") << e.what()
                         << mo_file_reader::gettext("L_exit_aplication") << std::endl;

						return 0; // no error for developer mode
				}
			}
			#endif

			assert(argm.count("port") && argm.count("rpc-port"));
			myserver_ptr = std::make_unique<c_tunserver>(argm.at("port").as<int>(), argm.at("rpc-port").as<int>());
			assert(myserver_ptr);
			auto& myserver = *myserver_ptr;
			myserver.set_desc(desc);

            _note("After devel/demo BoostPO code");

			// === argm now can contain options added/modified by developer mode ===
			po::notify(argm);  // !
			_note("After BoostPO notify");

			for(auto &arg: argm) _info("Argument in argm: " << arg.first );

			// --- debug level for main program ---
			bool is_debug=false;
			if (argm.count("debug") || argm.count("d")) is_debug=true;
			_note("Will we keep debug: is_debug="<<is_debug);

			g_dbg_level_set(config_default_basic_dbg_level, "For normal program run");
			if (is_debug) g_dbg_level_set(10,"For debug program run");
			if (argm.count("quiet") || argm.count("q")) g_dbg_level_set(200,"For quiet program run", true);
			_note("BoostPO after parsing debug");

			if (argm.count("help")) { // usage
				std::cout << *desc;
				std::cout << std::endl << project_version_info() << std::endl;
				return 0;
			}

			#if EXTLEVEL_IS_PREVIEW
			if (argm.count("set-IDI")) {
                    if (!argm.count("my-key")) { _erro( mo_file_reader::gettext("L_setIDI_require_myKey") );       return 1;       }

				auto name = argm["my-key"].as<std::string>();
				myserver.program_action_set_IDI(name);
				return 0; // <--- return
			}
			#endif

			_note("BoostPO before info");
			if (argm.count("info")) {
				if (!argm.count("my-key")) {
                    _erro( mo_file_reader::gettext("L_info_require_myKey") );

					return 1;
				}
				auto name = argm["my-key"].as<std::string>();
				antinet_crypto::c_multikeys_pub keys;
				keys.datastore_load(name);
				_info(keys.to_debug());
				return 0;
			}

			if (argm.count("list-my-keys")) {
				auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
				std::vector<std::string> keys = datastore::get_file_list(keys_path);
				std::string IDI_key = "";
			try {
				IDI_key = datastore::load_string(e_datastore_galaxy_instalation_key_conf, "IDI");
			} catch (std::invalid_argument &err) {
                    _dbg2(mo_file_reader::gettext("L_IDI_not_set_err"));

				}
                std::cout << mo_file_reader::gettext("L_your_key_list") << std::endl;

				for(auto &key_name : keys) {
					//remove .PRV extension
					size_t pos = key_name.find(".PRV");
					std::string actual_key = key_name.substr(0,pos);
					std::cout << actual_key << (IDI_key == actual_key ? " * IDI" : "") << std::endl;
				}
				return 0;
			}

			if (argm.count("gen-key")) {
				myserver.program_action_gen_key(argm);
				return 0;
			} // gen-key
			if (argm.count("gen-key-simple")) {
				myserver.program_action_gen_key_simple();
				return 0;
			} // gen-key
            #ifdef HTTP_DBG
            if (argm.count("http-dbg-port")) {
                http_dbg_port = argm["http-dbg-port"].as<int>();
            } //http-dbg-port
            #endif
			#if EXTLEVEL_IS_PREVIEW
			if (argm.count("sign")) {

				antinet_crypto::c_multikeys_PRV signing_key;
				std::string output_file;
				if (argm.count("my-key")) {
					output_file = argm["my-key"].as<std::string>();
					signing_key.datastore_load(output_file);

				} else if (argm.count("my-key-file")) {
					output_file = argm["my-key-file"].as<std::string>();
					sodiumpp::locked_string key_data(datastore::load_string_mlocked(e_datastore_local_path,
																					  output_file));
					signing_key.load_from_bin(key_data.get_string());

				} else {
					_erro("--my-key or --my-key-file option is required for --sign");
					return 1;
				}

				std::string to_sign_file;
				std::string to_sign;
				if (argm.count("sign-key")) {
					to_sign_file = argm["sign-key"].as<std::string>();
					to_sign = datastore::load_string(e_datastore_galaxy_pub, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					// adding ".pub" to make signature.pub.sig it's more clear (key.pub.sig is signature of key.pub)
					datastore::save_string(e_datastore_galaxy_sig, to_sign_file+".pub", sign.serialize_bin(), true);
				} else if (argm.count("sign-key-file")) {
					to_sign_file = argm["sign-key-file"].as<std::string>();
					to_sign = datastore::load_string(e_datastore_local_path, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					datastore::save_string(e_datastore_local_path, to_sign_file+".sig", sign.serialize_bin(), true);

				} else if (argm.count("sign-data-file")) {
					to_sign_file = argm["sign-data-file"].as<std::string>();
					to_sign = datastore::load_string(e_datastore_local_path, to_sign_file);
					auto sign = signing_key.multi_sign(to_sign);
					datastore::save_string(e_datastore_local_path, to_sign_file+".sig", sign.serialize_bin(), true);

				} else {
					_erro("-sign-key, sign-key-file or -sign-data-file option is required for --sign");
					return 1;
				}
				return 0;
			}

			_dbg1("BoostPO before verify");
			if(argm.count("verify")) {

				antinet_crypto::c_multikeys_pub trusted_key;
				std::string output_file;
				if (argm.count("trusted-key")) {
					output_file = argm["trusted-key"].as<std::string>();
					trusted_key.datastore_load(output_file);

				} else if (argm.count("trusted-key-file")) {
					output_file = argm["trusted-key-file"].as<std::string>();
					std::string key_data(datastore::load_string(e_datastore_local_path, output_file));
					trusted_key.load_from_bin(key_data);

				} else {
					_erro("--trusted-key or --trusted-key-file option is required for --verify");
					return 1;
				}

				bool extern_signature = false;
				antinet_crypto::c_multisign signature;

				// usefull local function
				auto load_signature = [&signature] (t_datastore stype, const std::string &filename) {
					std::string data (datastore::load_string(stype, filename));
					signature.load_from_bin(data);
				};

				std::string signature_file;
				if (argm.count("signature-file")) {
					extern_signature = true;
					signature_file = argm["signature-file"].as<std::string>();
					load_signature(e_datastore_local_path, signature_file);
				}

				std::string to_verify_file;
				std::string to_verify;
				if (argm.count("toverify-key")) {
					to_verify_file = argm["toverify-key"].as<std::string>();
					to_verify = datastore::load_string(e_datastore_galaxy_pub, to_verify_file);
					signature_file = to_verify_file + ".pub";
					if(!extern_signature)
						load_signature(e_datastore_galaxy_sig, signature_file);

				} else if (argm.count("toverify-key-file")) {
					to_verify_file = argm["toverify-key-file"].as<std::string>();
					to_verify = datastore::load_string(e_datastore_local_path, to_verify_file);
					signature_file = to_verify_file+".sig";
					if(!extern_signature)
						load_signature(e_datastore_local_path, signature_file);

				} else if (argm.count("toverify-data-file")) {
					to_verify_file = argm["toverify-data-file"].as<std::string>();
					to_verify = datastore::load_string(e_datastore_local_path, to_verify_file);
					signature_file = to_verify_file+".sig";
					if(!extern_signature)
						load_signature(e_datastore_local_path, signature_file);
				} else {
					_erro("-toverify-key, toverify-key-file or -sign-data-file option is required for --sign");
					return 1;
				}

			try {
				antinet_crypto::c_multikeys_pub::multi_sign_verify(signature,to_verify,trusted_key);
			} catch (std::invalid_argument &err) {
				_dbg2("Signature verification: fail");
				return 1;
			}
				_dbg2("Verify Success");
				return 0;
			}
			#endif

			_dbg1("BoostPO before config");
			#if EXTLEVEL_IS_PREVIEW
			if (argm.count("gen-config")) {
				c_json_genconf::genconf();
			}

			if (!(argm.count("no-config"))) {
				// loading peers from configuration file (default from galaxy.conf)
				_info("No no-config, will load config");
				std::string conf = argm["config"].as<std::string>();
				c_galaxyconf_load galaxyconf(conf);
				auto peer_refs = galaxyconf.get_peer_references();
				_info("Will add peer(s) from config file, count: " << peer_refs.size());
				for(auto &ref : peer_refs) {
					myserver.add_peer(ref);
				}
			}
			#endif
			_dbg1("BoostPO after config");

			// ------------------------------------------------------------------
			// end of options
			// ------------------------------------------------------------------

			_info("Configuring my own reference (keys):");

			bool have_keys_configured=false;
			try {
				auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
				std::vector<std::string> keys = datastore::get_file_list(keys_path);
				size_t have_keys = (keys.size() > 0);

				auto conf_IDI = datastore::get_full_path(e_datastore_galaxy_instalation_key_conf,"IDI");
				bool conf_IDI_ok = datastore::is_file_ok(conf_IDI);

				if (have_keys) {
					if (conf_IDI_ok) {
						have_keys_configured = true;
					} else {
						_warn("You have keys, but not IDI configured. Trying to make default IDI of your keys ...");
						_warn("If this warn still occurs, make sure you have backup of your keys");
						myserver.program_action_set_IDI("IDI");
						have_keys_configured = true;
					}
				}

			} catch(...) {
				_info("Can not load keys list or IDI configuration");
				have_keys_configured=0;
			}

			if (have_keys_configured) {
				bool ok=false;

				try {
					myserver.configure_mykey();
					ok=true;
				} UI_CATCH("Loading your key");

				if (!ok) {
					std::cout << "You seem to already have your hash-IP key, but I can not load it." << std::endl;
					std::cout << "Hint:\n"
						<< "You might want to move elsewhere current keys and create new keys (but your virtual-IP address will change!)"
						<< "Or maybe instead try other version of this program, that can load this key."
						<< std::endl
					;
					_throw_error( ui::exception_error_exit("There is existing IP-key but can not load it.") ); // <--- exit
				}
			} else {
				std::cout << "You have no ID keys yet - so will create new keys for you." << std::endl;

				auto step_make_default_keys = [&]()	{
					ui::action_info_ok("Generating your new keys.");
					const string IDI_name = myserver.program_action_gen_key_simple();
					myserver.program_action_set_IDI(IDI_name);
					ui::action_info_ok("Your new keys are created.");
					myserver.configure_mykey();
					ui::action_info_ok("Your new keys are ready to use.");
				};
				UI_EXECUTE_OR_EXIT( step_make_default_keys );

			}

			// ------------------------------------------------------------------

			string my_name = config_default_myname;
			if (argm.count("myname")) my_name = argm["myname"].as<string>();
						myserver.set_my_name(my_name);
						ui::action_info_ok(mo_file_reader::gettext("L_your_haship_address") + myserver.get_my_ipv6_nice());

			_info("Configuring my peers references (keys):");
			try {
				vector<string> peers_cmdline;
				try { peers_cmdline = argm["peer"].as<vector<string>>(); } catch(...) { }
				for (const string & peer_ref : peers_cmdline) {
					myserver.add_peer_simplestring( peer_ref );
				}
			} catch(...) {
                ui::action_error_exit(mo_file_reader::gettext("L_wrong_peer_typo"));

			}

			// ------------------------------------------------------------------
			myserver.set_argm(shared_ptr<po::variables_map>(new po::variables_map(argm)));
			auto peers_count = myserver.get_my_stats_peers_known_count();
			if (peers_count) {
				ui::action_info_ok("You will try to connect to up to " + std::to_string(peers_count) + " peer(s)");
			} else {
				string help_cmd1 = myserver.get_my_reference();
				ui::action_info_ok(mo_file_reader::gettext("L_no_other_computer_Option_for_other") + help_cmd1);
			}

		} // try parsing
		catch(ui::exception_error_exit) {
            std::cerr << mo_file_reader::gettext("L_exit_from_connect") << std::endl;

			return 1;
		}
		catch(po::error& e) {
            std::cerr << mo_file_reader::gettext("L_option_error") << e.what() << std::endl << std::endl;
			std::cerr << *desc << std::endl;
			return 1;
		}

//	} // try preparing
//	catch(std::exception& e) {
//        std::cerr << mo_file_reader::gettext("L_unhandled_exception")
//
//                    << e.what() << mo_file_reader::gettext("L_exit_aplication") << std::endl;
//
//		return 2;
//	}

	// ------------------------------------------------------------------
    _note(mo_file_reader::gettext("L_all_preparations_done"));

#ifdef HTTP_DBG
		_note(mo_file_reader::gettext("L_starting_httpdbg_server"));
		c_httpdbg_server httpdbg_server(http_dbg_port, *myserver_ptr);
		std::thread httpdbg_thread( [& httpdbg_server]() {
		httpdbg_server.run();
		}	);
#endif
		_note(mo_file_reader::gettext("L_starting_main_server"));
		myserver_ptr->run();
		_note(mo_file_reader::gettext("L_main_server_ended"));
#ifdef HTTP_DBG
		httpdbg_server.stop();
		httpdbg_thread.join(); // <-- for (also) making sure that main_httpdbg() will die before myserver will die
		_note(mo_file_reader::gettext("L_httpdbg_server_ended"));
#endif

	} // try running server
	catch(ui::exception_error_exit) {
        std::cerr << mo_file_reader::gettext("L_exiting_explained_above") << std::endl;

		return 1;
	}
	catch(std::exception& e) {
        std::cerr << mo_file_reader::gettext("L_unhandled_exception_running_server") << ' '
                  << e.what() << mo_file_reader::gettext("L_exit_aplication") << std::endl;

		return 2;
	}
	catch(...) {
        std::cerr << mo_file_reader::gettext("L_unknown_exception_running_server") << std::endl;

		return 3;
	}

	// ------------------------------------------------------------------
    _note(mo_file_reader::gettext("L_exit_no_error")); return 0;

}
