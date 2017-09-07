
#include "galaxysrv.hpp"
#include "libs1.hpp"

#include "netbuf.hpp"

#include "cable/simulation/cable_simul_addr.hpp"
#include "cable/udp/cable_udp_addr.hpp"
#include <cable/udp/cable_udp_obj.hpp>
#include "cable/simulation/cable_simul_obj.hpp"
#include "cable/simulation/world.hpp"
#include <cable/selector.hpp>

#include "libs0.hpp"
#include "cable/kind.hpp"

#include "tuntap/base/tuntap_base.hpp"
#include "tuntap/linux/c_tuntap_linux_obj.hpp"
#include "tuntap/windows/c_tuntap_windows.hpp"

#include <boost/filesystem.hpp> // for flag-file
#include <ipv6.hpp>

#include <boost/asio.hpp> // to create local address
#include <mutex.hpp>

#include <utils/privileges.hpp> // to drop/verify before main loop

// constexpr int cfg_jobs_tuntap_threads = 4;


void asiotest();

void c_galaxysrv::main_loop() {

	asiotest();

#if 0
	_goal("\n\nMain loop (new loop)\n\n");

	my_cap::verify_privileges_are_as_for_mainloop(); // confirm we are secured for the main loop

	auto world = make_shared<c_world>();
//	unique_ptr<c_cable_base_obj> cable = make_unique<c_cable_simul_obj>( world );
//	unique_ptr<c_cable_base_addr> peer_addr = make_unique<c_cable_simul_addr>( world->generate_simul_cable() );

	// ===========================================================================================================
	_clue("Allocating in main loop");
	size_t cfg_max_mtu = this->get_tuntap_mtu_current();
	size_t cfg_num_welds = 8;
	size_t cfg_weld_memsize = cfg_max_mtu * 3;
	{
		for (size_t i=0; i<cfg_num_welds; ++i) {
			_info("Creating weld #" << i);
			c_weld new_weld(cfg_weld_memsize);
			m_welds.push_back( std::move( new_weld  ) );
		}
		_mark("Allocations done");
		_note("Allocated " << m_welds.size() << " welds");
	}

	auto loop_exitwait = [&]() {
		string stopflag_name="/tmp/stop1";
		_fact("Running loop, create file " << stopflag_name << " to stop this loop.");
		boost::filesystem::remove( boost::filesystem::path(stopflag_name) );
		try {
			while (!m_exiting) {
				if (boost::filesystem::exists(stopflag_name)) break;
				std::this_thread::sleep_for( std::chrono::milliseconds(50) );
			}
		}
		catch (...) { _warn("The exitwait loop failed"); }
		this->start_exit(); // TODO-thread
	}; // lambda exitwait

	c_card_selector listen1_selector = [this]() -> auto {
		_fact("Starting to listen");
		// start listener:
		boost::asio::ip::udp::endpoint listen1_ep(boost::asio::ip::udp::v4(), get_default_galaxy_port()); // our local IP
		unique_ptr<c_cable_udp_addr> listen1 = make_unique<c_cable_udp_addr>( listen1_ep );
		c_card_selector listen1_selector( std::move(listen1) ); // will send from this my-address, to this peer
		_fact("Listen on " << listen1_selector );
		{
			UniqueLockGuardRW<MutexShared> lg( m_cable_cards.get_mutex() );
			m_cable_cards.get(lg).get_card(listen1_selector).listen_on(listen1_selector);
		}
		_goal("Listening on " << listen1_selector );
		return listen1_selector;
	} ();

	auto loop_tunread = [&](const int my_job_nr) {
		auto my_name = [=]() -> std::string {
			thread_local const string my_name_main = "[job#" + STR(my_job_nr) +"]";
			thread_local const string my_name_2 = my_name_main + " "s;
			return my_name_2;
		};

		try {
			_clue(my_name() + " starting");

			boost::asio::ip::udp::endpoint ep(boost::asio::ip::udp::v4(), get_default_galaxy_port()); // select our local source IP to use (and port)
			unique_ptr<c_cable_udp_addr> my_localhost = make_unique<c_cable_udp_addr>( ep );
			c_card_selector my_selector( std::move(my_localhost) ); // will send from this my-address, to this peer
			// pick up / create proper "card" (e.g. new socket from other my-address) and send from it:

			while (!m_exiting) {
			try {
				_dbg2(my_name() + "Reading TUN...");

				struct c_tuntap_read_result final {
					c_tuntap_read_result() = default;
					c_tuntap_read_result(c_tuntap_read_result &&) = default;
					c_tuntap_read_result(const c_tuntap_read_result &) = delete;

					c_tuntap_read_result(c_netchunk && _chunk) noexcept : chunk(std::move(_chunk)) {}

					c_netchunk chunk;
					c_haship_addr src_hip;
					c_haship_addr dst_hip;
				};

				auto func_find =
					[=](const c_weld & weld) {
						auto free_size = weld.get_free_size();
						_info("TESTING the weld "<<static_cast<const void*>(&weld)<<": free_size="<<free_size);
						if (free_size >= cfg_max_mtu) return true;
						return false;
					}
				;

				auto func_use = [=](c_weld & weld) -> c_tuntap_read_result {
					// receive the data into some part of the selected weld's buffer
					_note("USING the weld "<<static_cast<const void*>(&weld)<<" for tuntap read");

					size_t packet_max = cfg_max_mtu;
					size_t old_buffWrite = weld.m_bufWrite; // old write position in chunk, before we reserve

					_dbg1("Prepare tuntap read result");
					c_tuntap_read_result tuntap_result( // will store here result
						weld.m_buf.get_chunk( weld.m_bufWrite , packet_max) // find empty chunk, will return it as result
					);

					// *** block here and read from tuntap ***
					_dbg1("Tuntap - do the read...");
					size_t read_size_merit = m_tuntap.read_from_tun_separated_addresses(
						tuntap_result.chunk.data(), tuntap_result.chunk.size(),
						tuntap_result.src_hip, tuntap_result.dst_hip);
					_dbg1("Read tuntap merit=" << read_size_merit);
					if (read_size_merit == 0) _throw_error_runtime("Empty tun read");

					// adjust down size to actually used part of buffer
					tuntap_result.chunk.shrink_to( read_size_merit );
					weld.adjust_bufWrite( eint::eint_plus( old_buffWrite , read_size_merit ) );

					size_t read_ipv6size = ipv6_size_entireip_from_header( tuntap_result.chunk.to_tab_view() );
					size_t read_ipv6size_merit{ eint_minus( read_ipv6size , size_removed_from_merit ) };
					if (read_ipv6size_merit != read_size_merit) {
						_throw_error_runtime(join_string_sep("Invalid packet size: ipv6 headers:",read_ipv6size_merit,
						"tuntap driver", read_size_merit));
					}
					_info(my_name() + "TUN read: " << "src=" << tuntap_result.src_hip << " " << "dst=" << tuntap_result.dst_hip
						<< " TUN data: " << make_report(tuntap_result.chunk,20));
					return tuntap_result;
				};

				c_tuntap_read_result tuntap_result = m_welds.run_on_matching<c_tuntap_read_result>( func_find, func_use, 1 );

				// *** routing decision ***
				// TODO for now just send to first-cable of first-peer:
				auto const & peer_one_addr = m_peer.at(0)->m_reference.cable_addr.at(0); // what cable address to send to
				// TODO THREAD SAFE XXX TODO-NOW:
				// the reference should be used all under lock, probably, not just obtained under lock,
				// if the card is not thread-safe
				auto & door = m_cable_cards.use_RW( [&](auto & obj) -> auto& { return obj.get_card(my_selector); }  );

				// generate p2p:
				using proto = c_protocolv3; /// ^TODO move
				trivialserialize::generator gen(proto::max_header_size); // [[optimize]] remove alloc/free from gen(), use static buffer for it
				gen.push_byte_u( enum_to_int_safe<unsigned char>(proto::t_proto_cmd::e_proto_cmd_data_cart) );
				gen.push_bytes_n( g_ipv6_rfc::length_of_addr , to_binary_string(tuntap_result.src_hip) );
				gen.push_bytes_n( g_ipv6_rfc::length_of_addr , to_binary_string(tuntap_result.dst_hip) );
				// gen.push_bytes_n( crypto_box_NONCEBYTES , nonce_used.get().to_binary() ); // TODO avoid conversion/copy
				// gen.push_varstring( std::string(data, data+data_size)  ); // TODO view_string
				string header = gen.str_move();

				// send:
				c_cable_base_obj::t_asio_buffers_send buffers{
					{ header.data(), header.size() },
					{ tuntap_result.chunk.data(), tuntap_result.chunk.size() }
				};
				door.send_to( UsePtr(peer_one_addr) , buffers);

			} catch (const std::runtime_error &e) { _warn("Thread lambda (for tunread) got exception (but we can continue) - " << e.what());
			} catch (const std::exception &e) { _warn("Thread lambda (for tunread) got exception (but we can continue) - " << e.what());
				throw;
			}
			} // loop
			_note(my_name() + "Loop done - tun read");
		} catch (const std::exception &e) { _erro(my_name() + "Thread lambda (for tunread) got exception and EXITED - " << e.what()); }
		catch(...) { _warn(my_name() + "Thread-lambda (for tunread) got exception"); }
	}; // lambda tunread

	auto loop_cableread = [&]() {
		try {
			c_netbuf buf(9000);
			while (!m_exiting) {
				try{
					_dbg3("Reading cables...");
					c_card_selector his_door; // in c++17 instead, use "tie" with decomposition declaration

					// TODO@rfree TODO NOW XXX - lock cables like in vector_mutexed_obj, this lock it too wide:
					size_t read =	m_cable_cards.use_RW( [&](auto & obj) -> auto {
						return obj.get_card(listen1_selector).receive_from( his_door , buf.data() , buf.size() ); // ***********
					} );

					c_netchunk chunk( buf.data() , read ); // actually used part of buffer

					using proto = c_protocolv3;
					trivialserialize::parser parser( trivialserialize::parser::tag_caller_must_keep_this_buffer_valid(), reinterpret_cast<char*>(chunk.data()), chunk.size());
					proto::t_proto_cmd cmd = int_to_enum<proto::t_proto_cmd>(parser.pop_byte_u());
					c_haship_addr src_hip(c_haship_addr::tag_constr_by_addr_bin(), parser.pop_bytes_n(g_ipv6_rfc::length_of_addr));
					c_haship_addr dst_hip(c_haship_addr::tag_constr_by_addr_bin(), parser.pop_bytes_n(g_ipv6_rfc::length_of_addr));

					// what cable address to send to:
					c_cable_base_addr const & peer_one_addr = UsePtr( m_peer.at(0)->m_reference.cable_addr.at(0) );
					// TODO @rob: make full verification for different address prefixes like fd43
					auto check_ipv6_prefix = [this](const c_haship_addr &addr) {
						if (addr.at(0) != m_my_hip_prefix.at(0)) return false;
						if (addr.at(1) != m_my_hip_prefix.at(1)) return false;
						return true;
					}; // lambda

					const bool good_src_ipv6_prefix = check_ipv6_prefix(src_hip);
					const bool good_dst_ipv6_prefix = check_ipv6_prefix(dst_hip);
					if (!good_src_ipv6_prefix) {
						_info("Bad packet src prefix");
						continue;
					} else if (!good_dst_ipv6_prefix) {
						_info("Bad packet dst prefix");
						continue;
					}

					bool fwok = true;
					if ( peer_one_addr != his_door.get_my_addr() ) fwok=false;

					if (fwok) {
						_info("CABLE read, from " << his_door << make_report(chunk,20));
						size_t offset = sizeof(cmd) + 2*g_ipv6_rfc::length_of_addr;
						m_tuntap.send_to_tun_separated_addresses(chunk.data() + offset, chunk.size() - offset, src_hip, dst_hip);
						_info("Sent to tuntap");
					} else {
						_info("Ignoring packet from unexpected peer " << his_door << ", we wanted data from " << peer_one_addr );
					}
				}
				catch(const std::exception & ex) { _warn("Cannot handle this packet. Exception: " << ex.what()); }
				catch(...) { _warn("Cannot handle this packet. Unknown exception"); }
			} // loop
			_note("Loop done - cable reader");
		}
		catch(const std::exception & ex) { _warn("Thread-lambda loop_cableread got exception: " << ex.what()); }
		catch(...) { _warn("Thread-lambda loop_cableread got exception"); }
	}; // lambda cableread

	std::vector<unique_ptr<std::thread>> threads;


	threads.push_back( make_unique<std::thread>( loop_exitwait ) );

	for (int i=0; i<cfg_jobs_tuntap_threads; ++i) {
		_mark("Starting tuntap thread #" << i);
		threads.push_back( make_unique<std::thread>( loop_tunread , i ) );
	}
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
#endif

}

void c_galaxysrv::start_exit() {
	_goal("Start exiting");
	m_exiting=1;
	m_cable_cards.use_RW( [](auto & obj_cards) { obj_cards.stop_threadsafe(); } );
	_goal("Start exiting - ok");
}

uint16_t c_galaxysrv::get_tuntap_mtu_default() const {
	return 16*1024;
}

uint16_t c_galaxysrv::get_tuntap_mtu_current() const {
	return this->get_tuntap_mtu_default(); // TODO@rob + TODO@rfree instead actually query the card's MTU now set
}

void c_galaxysrv::init_tuntap() {
	assert(m_prefix_len != -1);
	m_tuntap.set_tun_parameters(get_my_hip(), m_prefix_len, this->get_tuntap_mtu_default());
}

// my key @new
void c_galaxysrv::configure_mykey(const std::string &ipv6_prefix) {
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
	my_IDI->set_ipv6_prefix(ipv6_prefix);
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
	m_my_hip_prefix.at(0) = m_my_hip.at(0);
	m_my_hip_prefix.at(1) = m_my_hip.at(1);
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

void c_galaxysrv::set_prefix_len(int prefix_len) {
	m_prefix_len = prefix_len;
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

