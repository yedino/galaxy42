
#include "the_program_newloop.hpp"
#include "platform.hpp"
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

#include "newloop.hpp"
#include <c_crypto.hpp>

#ifdef HTTP_DBG
	#include <thread>
	#include <mutex>
	#include <boost/asio.hpp>
	#include "httpdbg/httpdbg-server.hpp"
#endif

#include "galaxysrv.hpp" // ***
#include "galaxysrv_peers.hpp" // e.g. for help

// delete this block later:
#include "cable/base/cable_base_addr.hpp"
#include "cable/base/cable_base_obj.hpp"
#include "cable/simulation/cable_simul_addr.hpp"
#include "cable/simulation/cable_simul_obj.hpp"

#include "tuntap/base/tuntap_base.hpp"
#include "tuntap/linux/c_tuntap_linux_obj.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"
#include "tuntap/macosx/c_tuntap_macosx_obj.hpp"

#include "tunserver.hpp" // delete?


// --- for using crypto directly

#include "crypto/crypto.hpp"
#include "datastore.hpp"

#include "crypto/crypto_basic.hpp"

#if ENABLE_CRYPTO_NTRU
	#include "crypto/ntrupp.hpp"

	// ntru sign
	extern "C" {
#include <constants.h>
#include <pass_types.h>
#include <hash.h>
#include <ntt.h>
#include <pass.h>
	}

#endif

#if ENABLE_CRYPTO_SIDH
	#include "crypto/sidhpp.hpp"
#endif

#include <utils/privileges.hpp>
#include "test/special_behaviour/special_demo.hpp" ///< demo functions for tsan, ubsan

// -------------------------------------------------------------------

class c_the_program_newloop_pimpl {
	public:
		c_the_program_newloop_pimpl()=default;
		~c_the_program_newloop_pimpl()=default;

		unique_ptr<c_galaxysrv> server;

	private:
		friend class c_the_program_newloop;
};


c_the_program_newloop::c_the_program_newloop()
	: pimpl( new c_the_program_newloop_pimpl() )
{	}

c_the_program_newloop::~c_the_program_newloop() {
	_check( ! m_pimpl_deleted); // avoid double destruction of this object
	if (pimpl) { delete pimpl; }
	m_pimpl_deleted=true;
}

///@param level - memory use allowed: level=0 - basic tests,  level=1 also SIDH, level=2 also NTru
void test_create_cryptolink(const int number_of_test, int level) {
	using namespace antinet_crypto;

	for (std::remove_cv<decltype(number_of_test)>::type i = 0; i < number_of_test; ++i) {
		// g_dbg_level_set(160, "start test");
		c_multikeys_PAIR keypairA, keypairB;

		keypairA.generate(e_crypto_system_type_Ed25519, 2);
		keypairB.generate(e_crypto_system_type_Ed25519, 2);

#if ENABLE_CRYPTO_SIDH
		if (level>=1) {
			keypairA.generate(e_crypto_system_type_SIDH, 2);
			keypairB.generate(e_crypto_system_type_SIDH, 2);
		}
#endif

#if ENABLE_CRYPTO_NTRU
		if (level>=2) {
			keypairA.generate(e_crypto_system_type_NTRU_sign, 1);
			keypairB.generate(e_crypto_system_type_NTRU_sign, 1);
		}
#endif

		if (level>=0) _dbg2("Advanced crypto tests enabled");

		c_multikeys_pub keypubA = keypairA.m_pub;
		c_multikeys_pub keypubB = keypairB.m_pub;

		c_crypto_tunnel AliceCT(keypairA, keypubB, "Alice");
		AliceCT.create_IDe();
		string packetstart_1 = AliceCT.get_packetstart_ab(); // A--->>>
		c_crypto_tunnel BobCT(keypairB, keypubA, packetstart_1, "Bobby");
		string packetstart_2 = BobCT.get_packetstart_final(); // B--->>>
		AliceCT.create_CTf(packetstart_2); // A<<<---

		const std::string msg(1024, 'm');
		t_crypto_nonce nonce_used;
		auto msg_encrypted = AliceCT.box(msg, nonce_used);
		_check(msg != msg_encrypted);
		auto msg_decrypted = BobCT.unbox(msg_encrypted, nonce_used);
		_check(msg == msg_decrypted);
	}
}

void c_the_program_newloop::use_options_peerref() {
	_note("Configuring my peers references (keys):");
	try {
		vector<string> peers_cmdline;
		if(m_argm.count("peer")) {
			try {
				peers_cmdline = m_argm["peer"].as<vector<string>>();
			} catch(boost::bad_any_cast &err) {
			_warn(err.what());
			}
		}
		for (const string & peer_ref : peers_cmdline) {
			_info( peer_ref  );
			UsePtr(pimpl->server).add_peer_simplestring( peer_ref );
		}
	} catch(const std::exception &err) {
		_warn(err.what());
		ui::action_error_exit(mo_file_reader::gettext("L_wrong_peer_typo"));
	}
}


void c_the_program_newloop::programtask_load_my_keys() {
// ------------------------------------------------------------------
// @new - now in newloop new galaxysrv
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
						UsePtr(pimpl->server).program_action_set_IDI("IDI");
						have_keys_configured = true;
					}
				}

			} catch(...) {
				_info("Can not load keys list or IDI configuration");
				have_keys_configured=0;
			}

			const std::string ipv6_prefix = [this] {
				std::string ret = m_argm.at("set-prefix").as<std::string>();
				_check_input(ret.size() == 4);
				std::transform(ret.cbegin(), ret.cend(), ret.begin(),
					[](unsigned char c){return std::tolower(c);}
				);
				_check_input(ret.at(0) == 'f');
				_check_input(ret.at(1) == 'd');
				_check_input(ret.at(2) == '4');
				if (ret.at(3) == '2') UsePtr(pimpl->server).set_prefix_len(16);
				else if (ret.at(3) == '3') UsePtr(pimpl->server).set_prefix_len(24);
				else if (ret.at(3) == '4') throw std::invalid_argument("address reserved");
				else if (ret.at(3) == '5') throw std::invalid_argument("address reserved");
				else throw std::invalid_argument("address not supported");
				return ret;
			}(); // lambda

			if (have_keys_configured) {
				bool ok=false;

				try {
					UsePtr(pimpl->server).configure_mykey(ipv6_prefix);
					ok=true;
				} catch UI_CATCH("Loading your key");

				if (!ok) {
					_fact( "You seem to already have your hash-IP key, but I can not load it." );
					_fact( "Hint:\n"
						<< "You might want to move elsewhere current keys and create new keys (but your virtual-IP address will change!)"
						<< "Or maybe instead try other version of this program, that can load this key."
					);
					_throw_error( ui::exception_error_exit("There is existing IP-key but can not load it.") ); // <--- exit
				}
			} else {
				_fact( "You have no ID keys yet - so will create new keys for you." );

				auto step_make_default_keys = [&]() {
					ui::action_info_ok("Generating your new keys.");
					const string IDI_name = UsePtr(pimpl->server).program_action_gen_key_simple();
					UsePtr(pimpl->server).program_action_set_IDI(IDI_name);
					ui::action_info_ok("Your new keys are created.");
					UsePtr(pimpl->server).configure_mykey(ipv6_prefix);
					ui::action_info_ok("Your new keys are ready to use.");
				};
				UI_EXECUTE_OR_EXIT( step_make_default_keys );
			}
// ^ new ------------------------------------------------------------------
}

void c_the_program_newloop::programtask_tuntap() {

	pimpl->server->init_tuntap();
}


int c_the_program_newloop::run_special() {
	bool tainted=false;
	auto maybe_run_special = [&](const string & name, auto & func, bool do_taint) {
		if (m_argm.at(name).as<bool>()) {
			_goal("Run test: "<<name);
			if (do_taint) tainted=true;
			func();
		} else _info("NOT running test "<<name);
	};
	maybe_run_special("special-warn1", n_special_behaviour::example_warn_1, false);
	maybe_run_special("special-ubsan1", n_special_behaviour::example_ubsan_1, true);
	maybe_run_special("special-tsan1", n_special_behaviour::example_tsan_1, true);
	maybe_run_special("special-memcheck1", n_special_behaviour::example_memcheck_1, true);
	maybe_run_special("special-memcheck2", n_special_behaviour::example_memcheck_2, true);
	if (tainted) {
		_goal("After executing above tests, now the program is tainted, and must exit.");
		return 2; // exit code
	}
	return 0;
}

int c_the_program_newloop::main_execution() {
	PROGRAM_SECTION_TITLE;
	_mark("newloop main_execution");

	_clue("Setting debug level (main loop - new loop)");
	bool is_debug=false;
	if (m_argm.count("debug") || m_argm.count("d")) is_debug=true;
	_note("Will we keep debug: is_debug="<<is_debug);

	g_dbg_level_set(config_default_basic_dbg_level, "For normal program run");
	if (is_debug) g_dbg_level_set(10,"For debug program run");
	if (m_argm.count("dlevel")) {
		auto dlevel = int{  m_argm.at("dlevel").as<int>()  };
		if (dlevel != -1) {
			_note("Option --dlevel sets new level type: " << dlevel);
			g_dbg_level_set( dlevel , "Set by --dlevel" );
		}
	}

	{
		auto ret = this->run_special();
		if (ret) return ret;
	}

//	g_dbg_level_set(5, "Debug the newloop");

	pimpl->server = make_unique<c_galaxysrv>();

	this->programtask_load_my_keys();
	this->programtask_tuntap();
	my_cap::drop_privileges_after_tuntap(); // [security] ok we're done tuntap

	this->use_options_peerref();

	my_cap::drop_privileges_before_mainloop(); // [security] we do not need special privileges since we enter main loop now


	pimpl->server->main_loop();

	_mark("newloop main_execution - DONE");

	//	newloop_main( argt );
	return 0;
}

std::tuple<bool,int> c_the_program_newloop::programtask_help(const string & topic) {
	_info("Showing help for topic: " << topic);
	std::tuple<bool,int> ret_ok(true,0), ret_bad(false,1);
	if (topic=="peer") {
		_goal("Information about the peer format:");
		std::ostringstream oss;
		c_galaxysrv_peers::help_peer_ref(oss);
		_goal( oss.str() );
		return ret_ok;
	}
	return ret_bad;
}

std::tuple<bool,int> c_the_program_newloop::base_options_commands_run() {
	if (m_argm.count("helptopic")) {
		_note("there is a helptopic");
		string help_cmd;
		try {
			help_cmd = m_argm.at("helptopic").as<string>();
		} catch(...) { _throw_error_runtime("--helptopic option can not be read"); }
		return programtask_help(help_cmd);
	}

	if (m_argm.count("help")) {
		string help_cmd;
		try {
			help_cmd = m_argm.at("help").as<string>();
		} catch(...) { _throw_error_runtime("--help option can not be read"); }
		return c_the_program_tunserver::base_options_commands_run(); ///< basic commands, e.g. with basic --help
	}
	return std::tuple<bool,int>(false,0);
}

