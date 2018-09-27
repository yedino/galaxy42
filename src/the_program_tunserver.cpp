
#include "the_program_tunserver.hpp"
#include <platform.hpp>
#include "libs1.hpp"

#include "project.hpp"
#include <mo_reader.hpp>
#include "ui.hpp"
#include "trivialserialize.hpp"
#include "datastore.hpp"
#include "c_json_genconf.hpp"
#include "c_json_load.hpp"

#include "rpc/rpc.hpp"
#include "galaxy_debug.hpp"


#ifdef HTTP_DBG
#include <thread>
#include <mutex>
#include <boost/asio.hpp>
#include "httpdbg/httpdbg-server.hpp"
#endif

void c_the_program_tunserver::options_create_desc() {
	PROGRAM_SECTION_TITLE;
		namespace po = boost::program_options;
		unsigned line_length = 120;

		config_default_myname = "galaxy";

		m_boostPO_desc = make_shared<po::options_description>( mo_file_reader::gettext("L_options") , line_length);
		auto & desc = m_boostPO_desc;
		desc->add_options()
                    ("help", mo_file_reader::gettext("L_what_help_do").c_str())
                    ("helptopic", po::value<string>(), mo_file_reader::gettext("L_what_help_do").c_str())

                    ("h", mo_file_reader::gettext("L_what_h_do").c_str())

                    ("debug", mo_file_reader::gettext("L_what_debug_do").c_str())

                    ("insecure-cap", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (do not modify/drop CAP/capability)").c_str())

#ifdef ANTINET_linux
                    ("tun-missing-ok", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_tun-missing-ok")
											+ " [linux]").c_str())
#endif


                    ("special-warn1", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (show a _warn warning - use this in newloop; test is done after dropping CAP/capability)").c_str())
                    ("special-ubsan1", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (to test UBSAN: execute an UB signed overflow - use this in newloop; test is done after dropping CAP/capability)").c_str())
                    ("special-tsan1", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (to test TSAN: execute an concurent UB - use this in newloop; test is done after dropping CAP/capability)").c_str())
                    ("special-memcheck1", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (to test valgrind/memcheck: execute an invalid memory access - use this in newloop; test is done after dropping CAP/capability)").c_str())
                    ("special-memcheck2", po::value<bool>()->default_value(false), (mo_file_reader::gettext("L_options_insecure-ADVANCED")
											+ " (like memcheck1)").c_str())

                    ("d", mo_file_reader::gettext("L_what_d_do").c_str())
                    ("dlevel", po::value<int>()->default_value(-1), mo_file_reader::gettext("L_option_dlevel").c_str())

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
                        ("rpc-port", po::value<int>()->default_value(42000), mo_file_reader::gettext("L_rpcPort_do").c_str())
                        ("rpc-listen-address", po::value<std::string>()->default_value("127.0.0.1"), mo_file_reader::gettext("L_rpcListenAddress_do").c_str())
                    ("remove-peers", po::value<bool>()->default_value(false), mo_file_reader::gettext("L_remove_peers_do").c_str())
                    ("remove-peers-timeout", po::value<unsigned int>()->default_value(30), mo_file_reader::gettext("L_remove_peers_timeout_do").c_str())
										("home-env", mo_file_reader::gettext("L_what_home-env_option_do").c_str())

                     ("set-prefix", po::value<std::string>()->default_value("fd42"), mo_file_reader::gettext("L_setThePrefix").c_str())
//                     ("mask-len", po::value<unsigned short>()->default_value(16), mo_file_reader::gettext("L_setMaskLength").c_str())


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

//			("verify", "COMMAND: Verify key or data with trusted-key and key or data"
//					   "\nrequires [--trusted-key or --trusted-key-file and --toverify-key or --toverify-key-file "
//					   "or --toverify-data-file *--signature-file]"
//					   "\nDefault signature file name = key/data file name + \".sig\" extension"
//					   "in same location as key/data file")
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

        pfp_note(mo_file_reader::gettext("L_parse_program_option"));

}


void c_the_program_tunserver::options_multioptions() {
	PROGRAM_SECTION_TITLE;
	// option help is handled elsewhere

	const auto & argm = m_argm;
	pfp_UNUSED(argm);

			#if EXTLEVEL_IS_PREVIEW
			pfp_info("BoostPO Will parse demo/devel options");

			{ // Convert shortcut options:  "--demo foo"   ----->   "--devel --develdemo foo"
				auto opt_demo = argm["demo"].as<string>();
				if ( opt_demo!="" ) {
//					g_dbg_level_set(10,"Running in demo mode");
                                        g_dbg_level_set(10,"Running in demo mode");

//					pfp_info("The demo command line option is given:" << opt_demo);
                                        pfp_info("The demo command line option is given:" << opt_demo);

					// argm.insert(std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.at("develdemo") = po::variable_value( opt_demo , false );
					// (std::make_pair("develdemo", po::variable_value( opt_demo , false ))); // --devel --develdemo foo
					argm.insert(std::make_pair("devel",     po::variable_value( false , false ))); // --devel
				}
			}

			if (argm.count("devel")) { // can also set up additional options
				try {
					g_dbg_level_set(10,"Running in devel mode");
					pfp_info("The devel mode is active");

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
}

std::tuple<bool,int> c_the_program_tunserver::base_options_commands_run() {
	const auto & argm = m_argm;
	if (argm.count("help")) { // usage
		pfp_fact( *m_boostPO_desc );
		pfp_fact( std::endl << project_version_info() );
		return std::tuple<bool,int>(true,0);
	}
	return std::tuple<bool,int>(false,0);
}

int c_the_program_tunserver::main_execution() {
	PROGRAM_SECTION_TITLE;
	pfp_mark("Main execution of the old-loop");

	pfp_warn("Remember, that this old-loop code is NOT secured as new-loop code, e.g. is not droping CAP/root privileges!");
	{ using namespace std::chrono_literals;	std::this_thread::sleep_for(1s); }
	// ^ sleep to let user see this message clearly.

		try { // try parsing
			const auto & argm = m_argm;

			_check_user(argm.count("port") && argm.count("rpc-port"));
			pfp_fact("Will create a server");
			// *** creating the server object ***
			m_myserver_ptr = std::make_unique<c_tunserver>(
				argm.at("port").as<int>(),
				argm.at("rpc-port").as<int>(),
				argm
			);

			assert(m_myserver_ptr);
			auto& myserver = * m_myserver_ptr;
			myserver.set_desc(m_boostPO_desc);

			pfp_clue("After devel/demo BoostPO code");

			// --- debug level for main program ---
			pfp_clue("Setting debug level (main loop - old loop)");
			bool is_debug=false;
			if (argm.count("debug") || argm.count("d")) is_debug=true;
			pfp_note("Will we keep debug: is_debug="<<is_debug);

			g_dbg_level_set(config_default_basic_dbg_level, "For normal program run");
			if (is_debug) g_dbg_level_set(10,"For debug program run");
			if (argm.count("dlevel")) {
				auto dlevel = int{  argm.at("dlevel").as<int>()  };
				if (dlevel != -1) {
					pfp_note("Option --dlevel sets new level type: " << dlevel);
					g_dbg_level_set( dlevel , "Set by --dlevel" );
				}
			}
			if (argm.count("quiet") || argm.count("q")) g_dbg_level_set(200,"For quiet program run", true);
			pfp_note("BoostPO after parsing debug");


			#if EXTLEVEL_IS_PREVIEW
			if (argm.count("set-IDI")) {
                    if (!argm.count("my-key")) { pfp_erro( mo_file_reader::gettext("L_setIDI_require_myKey") );       return 1;       }

				auto name = argm["my-key"].as<std::string>();
				myserver.program_action_set_IDI(name);
				return 0; // <--- return
			}
			#endif

			pfp_note("BoostPO before info");
			if (argm.count("info")) {
				if (!argm.count("my-key")) {
                    pfp_erro( mo_file_reader::gettext("L_info_require_myKey") );

					return 1;
				}
				auto name = argm["my-key"].as<std::string>();
				antinet_crypto::c_multikeys_pub keys;
				keys.datastore_load(name);
				pfp_info(keys.to_debug());
				return 0;
			}

			if (argm.count("list-my-keys")) {
				auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
				std::vector<std::string> keys = datastore::get_file_list(keys_path);
				std::string IDI_key = "";
			try {
				IDI_key = datastore::load_string(e_datastore_galaxy_instalation_key_conf, "IDI");
			} catch (std::invalid_argument &err) {
                    pfp_dbg2(mo_file_reader::gettext("L_IDI_not_set_err"));

				}
//				std::cout << "Your key list:" << std::endl;
                pfp_fact( mo_file_reader::gettext("L_your_key_list") );

				for(auto &key_name : keys) {
					//remove .PRV extension
					size_t pos = key_name.find(".PRV");
					std::string actual_key = key_name.substr(0,pos);
					pfp_fact( actual_key << (IDI_key == actual_key ? " * IDI" : "") );
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
				if (argm.count("http-dbg-port")) m_http_dbg_port = argm["http-dbg-port"].as<int>();
			#endif

			if (argm.at("remove-peers").as<bool>()) {
				myserver.enable_remove_peers();
			}

			myserver.set_remove_peer_tometout(argm.at("remove-peers-timeout").as<unsigned int>());

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
					pfp_erro("--my-key or --my-key-file option is required for --sign");
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
					pfp_erro("-sign-key, sign-key-file or -sign-data-file option is required for --sign");
					return 1;
				}
				return 0;
			}

			pfp_dbg1("BoostPO before verify");
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
					pfp_erro("--trusted-key or --trusted-key-file option is required for --verify");
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
					pfp_erro("-toverify-key, toverify-key-file or -sign-data-file option is required for --sign");
					return 1;
				}

			try {
				antinet_crypto::c_multikeys_pub::multi_sign_verify(signature,to_verify,trusted_key);
			} catch (std::invalid_argument &err) {
				pfp_dbg2("Signature verification: fail");
				return 1;
			}
				pfp_dbg2("Verify Success");
				return 0;
			}
			#endif

			pfp_dbg1("BoostPO before config");
			#if EXTLEVEL_IS_PREVIEW
			if (argm.count("gen-config")) {
				c_json_genconf::genconf();
			}

			if (!(argm.count("no-config"))) {
				// loading peers from configuration file (default from galaxy.conf)
				pfp_info("No no-config, will load config");
				std::string conf = argm["config"].as<std::string>();
				c_galaxyconf_load galaxyconf(conf);
				auto peer_refs = galaxyconf.get_peer_references();
				pfp_info("Will add peer(s) from config file, count: " << peer_refs.size());
				for(auto &ref : peer_refs) {
					myserver.add_peer(ref);
				}
			}
			#endif
			pfp_dbg1("BoostPO after config");

			// ------------------------------------------------------------------
			// end of options
			// ------------------------------------------------------------------


// ------------------------------------------------------------------
// deprecated - now in newloop new galaxysrv
			pfp_info("Configuring my own reference (keys):");

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
						pfp_warn("You have keys, but not IDI configured. Trying to make default IDI of your keys ...");
						pfp_warn("If this warn still occurs, make sure you have backup of your keys");
						myserver.program_action_set_IDI("IDI");
						have_keys_configured = true;
					}
				}

			} catch(...) {
				pfp_info("Can not load keys list or IDI configuration");
				have_keys_configured=0;
			}
			const std::string ipv6_prefix = [this, &myserver] {
				std::string ret = m_argm.at("set-prefix").as<std::string>();
				_check_input(ret.size() == 4);
				std::transform(ret.cbegin(), ret.cend(), ret.begin(),
					[](unsigned char c){return std::tolower(c);}
				);
				_check_input(ret.at(0) == 'f');
				_check_input(ret.at(1) == 'd');
				_check_input(ret.at(2) == '4');
				if (ret.at(3) == '2') myserver.set_prefix_len(16);
				else if (ret.at(3) == '3') myserver.set_prefix_len(16);
				else if (ret.at(3) == '4') throw std::invalid_argument("address reserved");
				else if (ret.at(3) == '5') throw std::invalid_argument("address reserved");
				else throw std::invalid_argument("address not supported");
				return ret;
			}(); // lambda
			myserver.set_prefix(ipv6_prefix);
			if (have_keys_configured) {
				bool ok=false;

				try {
					myserver.configure_mykey(ipv6_prefix);
					ok=true;
				} catch UI_CATCH("Loading your key");

				if (!ok) {
					pfp_fact( "You seem to already have your hash-IP key, but I can not load it." );
					pfp_fact( "Hint:\n"
						<< "You might want to move elsewhere current keys and create new keys (but your virtual-IP address will change!)"
						<< "Or maybe instead try other version of this program, that can load this key."
					);
					pfp_throw_error( ui::exception_error_exit("There is existing IP-key but can not load it.") ); // <--- exit
				}
			} else {
				pfp_fact( "You have no ID keys yet - so will create new keys for you." );

				auto step_make_default_keys = [&]()	{
					ui::action_info_ok("Generating your new keys.");
					const string IDI_name = myserver.program_action_gen_key_simple();
					myserver.program_action_set_IDI(IDI_name);
					ui::action_info_ok("Your new keys are created.");
					myserver.configure_mykey(ipv6_prefix);
					ui::action_info_ok("Your new keys are ready to use.");
				};
				UI_EXECUTE_OR_EXIT( step_make_default_keys );
			}

// ^ deprecated ------------------------------------------------------------------


			// ------------------------------------------------------------------

			string my_name = config_default_myname;
			if (argm.count("myname")) my_name = argm["myname"].as<string>();
						myserver.set_my_name(my_name);
						ui::action_info_ok(mo_file_reader::gettext("L_your_haship_address") + myserver.get_my_ipv6_nice());

			pfp_info("Configuring my peers references (keys):");
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
			myserver.set_argm(shared_ptr<boost::program_options::variables_map>(new boost::program_options::variables_map(argm)));
			auto peers_count = myserver.get_my_stats_peers_known_count();
			if (peers_count) {
				ui::action_info_ok("You will try to connect to up to " + std::to_string(peers_count) + " peer(s)");
			} else {
				string help_cmd1 = myserver.get_my_reference();
				ui::action_info_ok(mo_file_reader::gettext("L_no_other_computer_Option_for_other") + help_cmd1);
			}

		} // try parsing
		catch(ui::exception_error_exit) {
//			std::cerr << "Exiting program now, as explained above..." << std::endl;
                        pfp_erro( mo_file_reader::gettext("L_exit_from_connect") );
			return 1;
		}
		catch(boost::program_options::error& e) {
			pfp_erro( mo_file_reader::gettext("L_option_error") << e.what() << std::endl );
			pfp_erro( *m_boostPO_desc );
			return 1;
		}

    pfp_note(mo_file_reader::gettext("L_all_preparations_done"));

		#ifdef HTTP_DBG
			pfp_note(mo_file_reader::gettext("L_starting_httpdbg_server"));
			c_httpdbg_server httpdbg_server(m_http_dbg_port, *m_myserver_ptr);
			std::thread httpdbg_thread( [& httpdbg_server]() {
				httpdbg_server.run();
			}	);
		#endif

		pfp_note(mo_file_reader::gettext("L_starting_main_server"));
		_check(m_myserver_ptr);
		pfp_goal("My server: calling run");
		m_myserver_ptr->run(); // <--- ENTERING THE MAIN LOOP (old loop) ***

		pfp_goal("My server: returned");
		pfp_note(mo_file_reader::gettext("L_main_server_ended"));

		#ifdef HTTP_DBG
			httpdbg_server.stop();
			httpdbg_thread.join(); // <-- for (also) making sure that main_httpdbg() will die before myserver will die
			pfp_note(mo_file_reader::gettext("L_httpdbg_server_ended"));
	#endif

	pfp_goal("Exiting normally the main part of program");
	return 0;
}

