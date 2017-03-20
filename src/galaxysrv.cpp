
#include "galaxysrv.hpp"
#include "libs1.hpp"

#include "netbuf.hpp"

#include "cable/simulation/cable_simul_addr.hpp"
#include "cable/udp/cable_udp_addr.hpp"
#include "cable/simulation/cable_simul_obj.hpp"
#include "cable/simulation/world.hpp"

#include "libs0.hpp"
#include "cable/kind.hpp"

#include "tuntap/base/tuntap_base.hpp"
#include "tuntap/linux/c_tuntap_linux_obj.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"

#include <boost/asio.hpp> // to create local address

void c_galaxysrv::main_loop() {
	_goal("\n\nMain loop\n\n");

	auto world = make_shared<c_world>();
//	unique_ptr<c_cable_base_obj> cable = make_unique<c_cable_simul_obj>( world );
//	unique_ptr<c_cable_base_addr> peer_addr = make_unique<c_cable_simul_addr>( world->generate_simul_cable() );


	auto loop_tunread = [&]() {
		try {
			c_netbuf buf(9000);
			string stopflag_name="/tmp/stop1";
			_fact("Running loop, create file " << stopflag_name << " to stop this loop.");
			while (1) {
				_dbg3("Reading TUN...");
				size_t read = m_tuntap.read_from_tun( buf.data(), buf.size() );
				c_netchunk chunk( buf.data() , read ); // actually used part of buffer
				_info("TUN read: " << make_report(chunk,20));
				// *** routing decision ***
				// TODO for now just send to first-cable of first-peer:
				auto const & peer_one_addr = m_peer.at(0)->m_reference.cable_addr.at(0); // what cable address to send to
				m_cable_cards.get_card(e_cable_kind_udp).send_to( UsePtr(peer_one_addr) , chunk.data() , chunk.size() );
				if (boost::filesystem::exists(stopflag_name)) break;
			} // loop
			_note("Loop done");
		} catch(...) { _warn("Thread-lambda got exception"); }
	}; // lambda tunread

	auto loop_cableread = [&]() {
		try {
			c_netbuf buf(9000);
			string stopflag_name="/tmp/stop1";
			_fact("Running loop, create file " << stopflag_name << " to stop this loop.");
			while (1) {
				_dbg3("Reading cables...");
				c_cable_udp_addr peer_addr;
				size_t read =	m_cable_cards.get_card(e_cable_kind_udp).receive_from( peer_addr , buf.data() , buf.size() ); // ***********
				c_netchunk chunk( buf.data() , read ); // actually used part of buffer
				_info("CABLE read, from " << peer_addr << make_report(chunk,20));
				m_tuntap.send_to_tun(chunk.data(), chunk.size());
				if (boost::filesystem::exists(stopflag_name)) break;
			} // loop
			_note("Loop done");
		} catch(...) { _warn("Thread-lambda got exception"); }
	}; // lambda cableread

	std::vector<unique_ptr<std::thread>> threads;


	c_cable_udp_addr address_all;
	address_all.init_addrdata( boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(),9042) );
	m_cable_cards.get_card(e_cable_kind_udp).listen_on( address_all );

	threads.push_back( make_unique<std::thread>( loop_tunread ) );
	threads.push_back( make_unique<std::thread>( loop_cableread ) );

	_goal("All threads started, count=" << threads.size());

	_fact("Joining all threads");
	for(auto & thr : threads) {
		_note("Joining thread");
		thr->join();
		_note("Joining thread - done");
	}
	_fact("After joining all threads");
	_goal("All threads joined, count=" << threads.size());
}

void c_galaxysrv::init_tuntap() {
	m_tuntap.set_tun_parameters(get_my_hip(), 16, 16000);
}

// my key @new
void c_galaxysrv::configure_mykey() {
	// creating new IDC from existing IDI // this should be separated
	//and should include all chain IDP->IDM->IDI etc.  sign and verification

	std::string IDI_name;
	try {
		IDI_name = datastore::load_string(e_datastore_galaxy_instalation_key_conf, "IDI");
	} catch (const std::invalid_argument &ex) {
		_info( join_string_sep("Can not find existing IDI",ex.what()) );
		throw stdplus::expected_not_found(); /// key doesn't exist at all then
	}

	std::unique_ptr<antinet_crypto::c_multikeys_PAIR> my_IDI;
	my_IDI = std::make_unique<antinet_crypto::c_multikeys_PAIR>();
	my_IDI->datastore_load_PRV_and_pub(IDI_name);
	// getting our address HIP from IDI
	auto IDI_ip_bin = my_IDI->get_ipv6_string_bin() ;
	auto IDI_ip_hexdot = my_IDI->get_ipv6_string_hexdot() ;
	try {
		try {
			if (static_cast<unsigned char>(IDI_ip_bin.at(0)) != 0xFD) throw std::exception();
			if (IDI_ip_hexdot.at(0) != 'f') throw std::exception();
			if (IDI_ip_hexdot.at(1) != 'd') throw std::exception();
		} catch(...) {
			std::ostringstream oss; oss<<"Your Hash-IP address looks not valid (not a Galaxy42 address?)"
				<< " - the IDI part is: ["<<IDI_ip_hexdot<<"]";
			const string msg_badip = oss.str();
			throw std::runtime_error(oss.str());
		}
	} catch UI_CATCH_RETHROW("Testing your Hash-IP (IDI)");

	c_haship_addr IDI_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDI_ip_hexdot );
	_info("IDI IPv6: " << IDI_ip_hexdot);
	_dbg1("IDI IPv6: " << IDI_hip << " (other var type)");

	// creating IDC for this session
	antinet_crypto::c_multikeys_PAIR my_IDC;
	my_IDC.generate(antinet_crypto::e_crypto_system_type_X25519,1);
	// signing it by IDI
	std::string IDC_pub_to_sign = my_IDC.m_pub.serialize_bin();
	antinet_crypto::c_multisign IDC_IDI_signature = my_IDI->multi_sign(IDC_pub_to_sign);

	// example veryifying
	antinet_crypto::c_multikeys_pub::multi_sign_verify(IDC_IDI_signature, IDC_pub_to_sign, my_IDI->m_pub);

	// save signature and IDI publickey in server
	m_my_IDI_pub = my_IDI->m_pub;
	m_IDI_IDC_sig = IDC_IDI_signature;

	// remove IDP from RAM
	// (use of locked_string should take care of actually shreding memory)
	my_IDI.reset(nullptr);

	// for debug, hip from IDC
	auto IDC_ip_hexdot = my_IDC.get_ipv6_string_hexdot() ;

	try {
		try {
			if (IDC_ip_hexdot.at(0) != 'f') throw std::exception();
			if (IDC_ip_hexdot.at(1) != 'd') throw std::exception();
		}
		catch (...) {
			std::ostringstream oss; oss<<"Your Hash-IP address looks not valid (not a Galaxy42 address?)"
				<< " - the IDC part is: ["<<IDC_ip_hexdot<<"]";
			const string msg_badip = oss.str();
		}
	} catch UI_CATCH_RETHROW("Testing your Hash-IP (IDC)");

	c_haship_addr IDC_hip = c_haship_addr( c_haship_addr::tag_constr_by_addr_dot() , IDC_ip_hexdot );
	_info("IDC IPv6: " << IDC_ip_hexdot);
	_dbg1("IDC IPv6: " << IDC_hip << " (other var type)");
	_note("Your temporary, current crypto key (IDC) is: " << IDC_ip_hexdot << " (this is just for information)");
	_goal("Your permanent IPv6 address (IDI) is: " << IDI_ip_hexdot << " - others can connect to this IP address to reach you");
	string IDI_as_url = "http://" + IDI_ip_hexdot ;
	_goal("For example your web page would be at: " << IDI_as_url );
	// now we can use hash ip from IDI and IDC for encryption
	m_my_hip = IDI_hip;
	m_my_IDC = my_IDC;
}

c_haship_addr c_galaxysrv::get_my_hip() const {
	return m_my_hip;
}

void c_galaxysrv::program_action_set_IDI(const string & keyname) {
	_note("Action: set IDI");
	_info("Setting the name of IDI key to: " << keyname);
	auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
	auto keys = datastore::get_file_list(keys_path);
	bool found = false;
	for (auto &key_name : keys) {
		//remove .PRV extension
		size_t pos = key_name.find(".PRV");
		std::string act = key_name.substr(0,pos);
		if (keyname == act) {	found = true;	break; }
	}
	if (found == false) {
		_erro("Can't find key (" << keyname << ") in your key list, so can't set it as IDI.");
	}
	_info("Key found ("<< keyname <<") and set as IDI");
	datastore::save_string(e_datastore_galaxy_instalation_key_conf,"IDI", keyname, true);
}

std::string c_galaxysrv::program_action_gen_key_simple() {
	const string IDI_name = "IDI";
//	ui::action_info_ok("Generating your new keys.");
        ui::action_info_ok(mo_file_reader::gettext("L_generatin_new_keys"));

	std::vector<std::pair<antinet_crypto::t_crypto_system_type,int>> keys; // list of key types
	keys.emplace_back(std::make_pair(antinet_crypto::t_crypto_system_type_from_string("ed25519"), 1));
	auto output_file = IDI_name;
	generate_crypto::create_keys(output_file, keys, true); // ***
	return IDI_name;
}

void c_galaxysrv::program_action_gen_key(const boost::program_options::variables_map & argm) {
	_note("Action: gen key");
	if (!argm.count("key-type")) {
		_throw_error( std::invalid_argument("--key-type option is required for --gen-key") );
	}

	std::vector<std::pair<antinet_crypto::t_crypto_system_type,int>> keys;
	auto arguments = argm["key-type"].as<std::vector<std::string>>();
	for (auto argument : arguments) {
		_dbg1("parse argument " << argument);
		std::replace(argument.begin(), argument.end(), ':', ' ');
		std::istringstream iss(argument);
		std::string str;
		iss >> str;
		_dbg1("type = " << str);
		antinet_crypto::t_crypto_system_type type = antinet_crypto::t_crypto_system_type_from_string(str);
		iss >> str;
		assert(str[0] == 'x');
		str.erase(str.begin());
		int number_of_keys = std::stoi(str);
		_dbg1("number_of_keys" << number_of_keys);
		keys.emplace_back(std::make_pair(type, number_of_keys));
	}

	std::string output_file;
	if (argm.count("new-key")) {
		output_file = argm["new-key"].as<std::string>();
		generate_crypto::create_keys(output_file, keys, true); // ***
	} else if (argm.count("new-key-file")) {
		output_file = argm["new-key-file"].as<std::string>();
		generate_crypto::create_keys(output_file, keys, false); // ***
	} else {
		_throw_error( std::invalid_argument("--new-key or --new-key-file option is required for --gen-key") );
	}
}

