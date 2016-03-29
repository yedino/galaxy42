/**
Copyrighted (C) 2016, GPL v3 Licence (may include also other code)
See LICENCE.txt
*/

const char * disclaimer = "*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere! ***"; // XXX

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <boost/program_options.hpp>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <string.h>
#include <assert.h>

#include <thread>

#include <cstring>

#include <sodium.h>

#include "libs1.hpp"
#include "counter.hpp"
#include "cpputils.hpp"

// linux (and others?) select use:
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

// for low-level Linux-like systems TUN operations
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include<netinet/ip_icmp.h>   //Provides declarations for icmp header
#include<netinet/udp.h>   //Provides declarations for udp header
#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header
// #include <net/if_ether.h> // peer over eth later?
// #include <net/if_media.h> // ?

#include "../NetPlatform.h" // from cjdns

// #include <net/if_tap.h>
#include <linux/if_tun.h>

#include "c_ip46_addr.hpp"
#include "c_peering.hpp"

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------



namespace developer_tests {

bool wip_strings_encoding(boost::program_options::variables_map & argm) {
	_mark("Tests of string encoding");
	string s1,s2,s3;
	using namespace std;
	s1="4a4b4c4d4e"; // in hex
//	s2="ab"; // in b64
	s3="y"; // in bin


	// TODO assert is results are as expected!
	// TODO also assert that the exceptions are thrown as they should be, below

	auto s1_hex = string_as_hex( s1 );
	c_haship_pubkey pub1( s1_hex );
	_info("pub = " << to_string(pub1));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex("4"))));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex("f4b4c4d4e"))));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex("4a4b4c4d4e"))));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex(""))));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex("ffffffff"))));
	_info("pub = " << to_string(c_haship_pubkey(string_as_hex("00000000"))));
	try {
		_info("pub = " << to_string(c_haship_pubkey(string_as_hex("4a4b4c4d4eaba46381826328363782917263521719badbabdbadfade7455467383947543473839474637293474637239273534873"))));
	} catch (std::exception &e) { _note("Test failed, as expected: " << e.what()); }
	try {
		_info("pub = " << to_string(c_haship_pubkey(string_as_hex("0aq"))));
	} catch (std::exception &e) { _note("Test failed, as expected: " << e.what()); }
	try {
		_info("pub = " << to_string(c_haship_pubkey(string_as_hex("qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"))));
	} catch (std::exception &e) { _note("Test failed, as expected: " << e.what()); }

//	c_haship_pubkey pub2( string_as_b64( s1 ) );
//	c_haship_pubkey pub3( string_as_bin( s1 ) );

	_info("Test done");
	return false;
}

} // namespace


// ------------------------------------------------------------------

class c_tunserver {
	public:
		c_tunserver();
		void configure_mykey_from_string(const std::string &mypub, const std::string &mypriv);
		void run(); ///< run the main loop
		void add_peer(const t_peering_reference & peer_ref); ///< add this as peer

		void help_usage() const; ///< show help about usage of the program

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop
		void wait_for_fd_event(); ///< waits for event of I/O being ready, needs valid m_tun_fd and others, saves the fd_set into m_fd_set_data

		c_haship_addr parse_tun_ip_src_dest(const char *buff, size_t buff_size);

		void peering_ping_all_peers();
		void debug_peers();

	private:
		int m_tun_fd; ///< fd of TUN file

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		typedef std::map< c_haship_addr, unique_ptr<c_peering> > t_peers_by_haship; ///< my peers (we always know their IPv6 - we assume here)
		t_peers_by_haship m_peer; ///< my peers

		c_haship_pubkey m_haship_pubkey; ///< pubkey of my IP
		c_haship_addr m_haship_addr; ///< my haship addres
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_tunserver::c_tunserver()
 : m_tun_fd(-1), m_sock_udp(-1)
{
}

// my key
void c_tunserver::configure_mykey_from_string(const std::string &mypub, const std::string &mypriv) {
	m_haship_pubkey = string_as_bin( string_as_hex( mypub ) );
	m_haship_addr = c_haship_addr( c_haship_addr::tag_constr_by_hash_of_pubkey() , m_haship_pubkey );
	_info("Configuring the router, I am: pubkey="<<to_string(m_haship_pubkey)<<" ip="<<to_string(m_haship_addr));
}

// add peer
void c_tunserver::add_peer(const t_peering_reference & peer_ref) { ///< add this as peer
	_note("Adding peer from reference=" << peer_ref
		<< " that reads: " << "peering-address=" << peer_ref.peering_addr << " pubkey=" << to_string(peer_ref.pubkey) << " haship_addr=" << to_string(peer_ref.haship_addr) );
	auto peering_ptr = make_unique<c_peering_udp>(peer_ref);
	// TODO(r) check if duplicated peer (map key) - warn or ignore dep on parameter
	m_peer.emplace( std::make_pair( peer_ref.haship_addr ,  std::move(peering_ptr) ) );
}

void c_tunserver::help_usage() const {
	// TODO(r) remove, using boost options
}

void c_tunserver::prepare_socket() {
	m_tun_fd = open("/dev/net/tun", O_RDWR);
	assert(! (m_tun_fd<0) );

  as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TUN; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, (void *)&ifr);
	if (errcode_ioctl < 0)_throw( std::runtime_error("Error in ioctl")); // TODO

	_mark("Allocated interface:" << ifr.ifr_name);

	{
		uint8_t address[16];
		for (int i=0; i<16; ++i) address[i] = m_haship_addr.at(i);
		// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
		// ...to the device to which we are setting IP address here:
		assert(address[0] == 0xFD);
		assert(address[1] == 0x42);
		NetPlatform_addAddress(ifr.ifr_name, address, 16, Sockaddr_AF_INET6);
	}

	// create listening socket
	m_sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	_assert(m_sock_udp >= 0);

	int port = 9042;
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(port);

	{
		int bind_result = -1;
		if (address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv4) {
			sockaddr_in addr4 = address_for_sock.get_ip4();
			bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
		}
		else if(address_for_sock.get_ip_type() == c_ip46_addr::t_tag::tag_ipv6) {
			sockaddr_in6 addr6 = address_for_sock.get_ip6();
			bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr6), sizeof(addr6));  // reinterpret allowed by Linux specs
		}
			_assert( bind_result >= 0 ); // TODO change to except
			_assert(address_for_sock.get_ip_type() != c_ip46_addr::t_tag::tag_none);
	}
	_info("Bind done - listening on UDP on: "); // TODO  << address_for_sock
}

void c_tunserver::wait_for_fd_event() { // wait for fd event
	_info("Selecting");
	// set the wait for read events:
	FD_ZERO(& m_fd_set_data);
	FD_SET(m_sock_udp, &m_fd_set_data);
	FD_SET(m_tun_fd, &m_fd_set_data);

	auto fd_max = std::max(m_tun_fd, m_sock_udp);
	_assert(fd_max < std::numeric_limits<decltype(fd_max)>::max() -1); // to be more safe, <= would be enough too
	_assert(fd_max >= 1);

	auto select_result = select( fd_max+1, &m_fd_set_data, NULL, NULL,NULL); // <--- blocks
	_assert(select_result >= 0);
}

c_haship_addr c_tunserver::parse_tun_ip_src_dest(const char *buff, size_t buff_size) { // TODO
	// vuln-TODO(u) throw on invalid size + assert

	assert(buff_size > 28 + INET6_ADDRSTRLEN                 + 8); // quick check. +8 to be sure ;)

	char ipv6_str[INET6_ADDRSTRLEN];
	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + 12, ipv6_str, INET6_ADDRSTRLEN);
	_dbg1("src ipv6_str " << ipv6_str);

	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + 28, ipv6_str, INET6_ADDRSTRLEN);
	_dbg1("dst ipv6_str " << ipv6_str);

	c_haship_addr x(c_haship_addr::tag_constr_by_addr_string(), ipv6_str);
	return x;
}

void c_tunserver::peering_ping_all_peers() {
	_info("Sending ping to all peers");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived

		// [protocol] build raw
		string_as_bin cmd_data; 
		cmd_data.bytes += string_as_bin( m_haship_pubkey ).bytes; 
		cmd_data.bytes += ";";
		peer_udp->send_data_udp_cmd(c_protocol::e_proto_cmd_public_hi, cmd_data, m_sock_udp);
	}
}

void c_tunserver::debug_peers() {
	_note("=== Debug peers ===");
	for(auto & v : m_peer) { // to each peer
		auto & target_peer = v.second;
		_info("  * Known peer on key [ " << string_as_dbg( v.first ).get() << " ] => " << (* target_peer) );
	}
}



void c_tunserver::event_loop() {
	_info("Entering the event loop");
	c_counter counter(2,true);
	c_counter counter_big(10,false);

	fd_set fd_set_data;


	this->peering_ping_all_peers();

	const int buf_size=65536;
	char buf[buf_size];


	while (1) {
		debug_peers();

		wait_for_fd_event();

		try { // ---

		if (FD_ISSET(m_tun_fd, &m_fd_set_data)) { // data incoming on TUN - send it out to peers
			auto size_read = read(m_tun_fd, buf, sizeof(buf)); // read data from TUN

			_info("TUN read " << size_read << " bytes: [" << string(buf,size_read)<<"]");
			try {
				auto & target_peer = m_peer.begin()->second;
				auto peer_udp = unique_cast_ptr<c_peering_udp>( target_peer ); // upcast to UDP peer derived
				c_haship_addr dst_hip = parse_tun_ip_src_dest(buf, size_read);
				_info("Destination: " << dst_hip);
				peer_udp->send_data_udp(buf, size_read, m_sock_udp);
			} catch(std::exception &e) {
				_warn("Can not send to peer, because:" << e.what()); // TODO more info (which peer, addr, number)
			} catch(...) {
				_warn("Can not send to peer (unknown)"); // TODO more info (which peer, addr, number)
			}
		}
		else if (FD_ISSET(m_sock_udp, &m_fd_set_data)) { // data incoming on peer (UDP) - will route it or send to our TUN
			sockaddr_in6 from_addr_raw; // peering address of peer (socket sender), raw format
			socklen_t from_addr_raw_size; // ^ size of it

			c_ip46_addr peer_ip; // IP of peer who sent it

			// ***
			from_addr_raw_size = sizeof(from_addr_raw); // IN/OUT parameter to recvfrom, sending it for IN to be the address "buffer" size
			auto size_read = recvfrom(m_sock_udp, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
			// ^- reinterpret allowed by linux specs (TODO)
			// sockaddr *src_addr, socklen_t *addrlen);

			if (from_addr_raw_size == sizeof(sockaddr_in6)) { // the message arrive from IP pasted into sockaddr_in6 format
				_erro("NOT IMPLEMENTED yet - recognizing IP of ipv6 peer"); // peeripv6-TODO(r)(easy)
				// trivial
			}
			else if (from_addr_raw_size == sizeof(sockaddr_in)) { // the message arrive from IP pasted into sockaddr_in (ipv4) format
				sockaddr_in addr = * reinterpret_cast<sockaddr_in*>(& from_addr_raw); // mem-cast-TODO(p) confirm reinterpret
				peer_ip.set_ip4(addr);
			} else {
				throw std::runtime_error("Data arrived from unknown socket address type");
			}

			_info("UDP Socket read from IP = " << peer_ip <<", size " << size_read << " bytes: " << string_as_dbg( string_as_bin(buf,size_read)).get());
			// ------------------------------------

			if (! (size_read >= 2) ) { _warn("INVALIDA DATA, size_read="<<size_read); continue; } // !
			int proto_version = static_cast<int>( static_cast<unsigned char>(buf[0]) ); // TODO
			_assert(proto_version >= c_protocol::current_version ); // let's assume we will be backward compatible (but this will be not the case untill official stable version probably)
			c_protocol::t_proto_cmd cmd = static_cast<c_protocol::t_proto_cmd>( buf[1] );
			if (cmd == c_protocol::e_proto_cmd_tunneled_data) {

				static unsigned char generated_shared_key[crypto_generichash_BYTES] = {43, 124, 179, 100, 186, 41, 101, 94, 81, 131, 17,
								198, 11, 53, 71, 210, 232, 187, 135, 116, 6, 195, 175,
								233, 194, 218, 13, 180, 63, 64, 3, 11};

				static unsigned char nonce[crypto_aead_chacha20poly1305_NPUBBYTES] = {148, 231, 240, 47, 172, 96, 246, 79};
				static unsigned char additional_data[] = {1, 2, 3};
				static unsigned long long additional_data_len = 3;
				// TODO randomize this data

				std::unique_ptr<unsigned char []> decrypted_buf (new unsigned char[size_read + crypto_aead_chacha20poly1305_ABYTES]);
				unsigned long long decrypted_buf_len;

				assert(crypto_aead_chacha20poly1305_KEYBYTES <= crypto_generichash_BYTES);

				// reinterpret the char from IO as unsigned-char as wanted by crypto code
				unsigned char * ciphertext_buf = reinterpret_cast<unsigned char*>( buf ) + 2; // TODO calculate depending on version, command, ...
				assert( size_read >= 3 );  // headers + anything
				long long ciphertext_buf_len = size_read - 2; // TODO 2 = hesder size
				assert( ciphertext_buf_len >= 1 );

				int r = crypto_aead_chacha20poly1305_decrypt(
					decrypted_buf.get(), & decrypted_buf_len,
					nullptr,
					ciphertext_buf, ciphertext_buf_len,
					additional_data, additional_data_len,
					nonce, generated_shared_key);
				if (r == -1) {
					_warn("verification fails");
					continue; // skip this packet (main loop)
				}

				// reinterpret for debug
				_info("UDP received, sending to TUN:" << decrypted_buf_len << " bytes: [" << string( reinterpret_cast<char*>(decrypted_buf.get()), decrypted_buf_len)<<"]" );
				write(m_tun_fd, decrypted_buf.get(), decrypted_buf_len);

			} // e_proto_cmd_tunneled_data
			else if (cmd == c_protocol::e_proto_cmd_public_hi) {
				_mark("Command HI received");
				int offset1=2; assert( size_read >= offset1);  string_as_bin cmd_data( buf+offset1 , size_read-offset1); // buf -> bin for comfortable use

				auto pos1 = 32; // [protocol] size of public key
				if (cmd_data.bytes.at(pos1)!=';') throw std::runtime_error("Invalid protocol format, missing coma"); // [protocol]
				string_as_bin bin_his_pubkey( cmd_data.bytes.substr(0,pos1) );
				_info("We received pubkey=" << string_as_dbg( bin_his_pubkey ).get() );
				t_peering_reference his_ref( peer_ip , string_as_bin( bin_his_pubkey ) );
				add_peer( his_ref );
			}
			else {
				_warn("Unknown protocol command, cmd="<<cmd);
				continue; // skip this packet (main loop)
			}
			// ------------------------------------

		}
		else _erro("No event selected?!"); // TODO throw

		}
		catch (std::exception &e) {
			_warn("Parsing network data caused an exception: " << e.what());
		}

// stats-TODO(r) counters
//		int sent=0;
//		counter.tick(sent, std::cout);
//		counter_big.tick(sent, std::cout);
	}
}

void c_tunserver::run() {
	std::cout << "Stating the TUN router." << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------

namespace developer_tests {

string make_pubkey_for_peer_nr(int peer_nr) {
	string peer_pub = string("4a4b4c") + string("4") + string(1, char('0' + peer_nr)  );
	return peer_pub;
}

bool wip_galaxy_route_star(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	std::cerr << "Running in developer mode. " << std::endl;

	const int node_nr = argm["develnum"].as<int>();  assert( (node_nr>=1) && (node_nr<=254) );
	std::cerr << "Running in developer mode - as node_nr=" << node_nr << std::endl;
	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");

	int peer_nr = node_nr==1 ? 2 : 1;
	string peer_pub = make_pubkey_for_peer_nr( peer_nr );
	string peer_ip = string("192.168.") + std::to_string( peer_nr  ) + string(".62"); // each connect to node .1., except the node 1 that connects to .2.

	_mark("Developer: adding peer with arguments: ip=" << peer_ip << " pub=" << peer_pub );

	// argm.insert(std::make_pair("K", po::variable_value( int(node_nr) , false )));
	argm.insert(std::make_pair("peerip", po::variable_value( peer_ip , false )));
	argm.at("peerpub") = po::variable_value( peer_pub , false );
	argm.at("mypub") = po::variable_value( make_pubkey_for_peer_nr(node_nr)  , false );
	return true; // continue the test
	// TODO@r finish auto deployment --r
}



} // namespace developer_tests

bool run_mode_developer(boost::program_options::variables_map & argm) {
	return developer_tests::wip_galaxy_route_star(argm);
}

int main(int argc, char **argv) {
	std::cerr << std::endl << disclaimer << std::endl << std::endl;


/*	c_ip46_addr addr;
	std::cout << addr << std::endl;
	struct sockaddr_in sa;
	inet_pton(AF_INET, "192.0.2.33", &(sa.sin_addr));
	sa.sin_family = AF_INET;
	addr.set_ip4(sa);
	std::cout << addr << std::endl;

	struct sockaddr_in6 sa6;
	inet_pton(AF_INET6, "fc72:aa65:c5c2:4a2d:54e:7947:b671:e00c", &(sa6.sin6_addr));
	sa6.sin6_family = AF_INET6;
	addr.set_ip6(sa6);
	std::cout << addr << std::endl;
*/
	c_tunserver myserver;
	try {
		namespace po = boost::program_options;
		po::options_description desc("Options");
		desc.add_options()
			("help", "Print help messages")
			("devel","Test: used by developer to run current test")
			("develnum", po::value<int>()->default_value(1), "Test: used by developer to set current node number (makes sense with option --devel)")
			// ("K", po::value<int>()->required(), "number that sets your virtual IP address for now, 0-255")
			("mypub", po::value<std::string>()->default_value("") , "your public key (give any string, not yet used)")
			("mypriv", po::value<std::string>()->default_value(""), "your PRIVATE key (give any string, not yet used - of course this is just for tests)")
			("peerip", po::value<std::string>()->required(), "IP over existing networking to connect to your peer")
			("peerpub", po::value<std::string>()->default_value(""), "public key of your peer");

		po::variables_map argm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), argm);
			cout << "devel" << endl;
			if (argm.count("devel")) {
				try {
					bool should_continue = run_mode_developer(argm);
					if (!should_continue) return 0;
				}
				catch(std::exception& e) {
				    std::cerr << "Unhandled Exception reached the top of main: (in DEVELOPER MODE)" << e.what() << ", application will now exit" << std::endl;
						return 0; // no error for developer mode
				}
			}
			// argm now can contain options added/modified by developer mode
			po::notify(argm);


			if (argm.count("help")) {
				std::cout << desc;
				return 0;
			}
			myserver.configure_mykey_from_string(
				argm["mypub"].as<std::string>() ,
				argm["mypriv"].as<std::string>()
			);
			myserver.add_peer( t_peering_reference(
				argm["peerip"].as<std::string>(),
				string_as_hex(	argm["peerpub"].as<std::string>() )
			) );
		}
		catch(po::error& e) {
			std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
			std::cerr << desc << std::endl;
			return 1;
		}
	}
	catch(std::exception& e) {
		    std::cerr << "Unhandled Exception reached the top of main: "
				<< e.what() << ", application will now exit" << std::endl;
		return 2;
	}

	myserver.run();
}


