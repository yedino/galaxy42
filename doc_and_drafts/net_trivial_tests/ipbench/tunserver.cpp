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



// declare sizes; also forward declarations
constexpr int g_haship_addr_size = 16;
constexpr int g_haship_pubkey_size = 32;
struct c_haship_addr;
struct c_haship_pubkey;

/***
@class virtual hash-ip, e.g. ipv6, usable for ipv6-cjdns (fc00/8), and of course also for our ipv6-galaxy (fd42/16)
*/
struct c_haship_addr : public std::array<unsigned char, g_haship_addr_size> { 
	c_haship_addr(); 
	c_haship_addr( const c_haship_pubkey & pubkey ); ///< create the IP address that matches given public key (e.g. hash of it)
};

c_haship_addr::c_haship_addr() : std::array<unsigned char, g_haship_addr_size>({}) { }
c_haship_addr::c_haship_addr( const c_haship_pubkey & pubkey ) : std::array<unsigned char, g_haship_addr_size>({}) { }

struct c_haship_pubkey : std::array<unsigned char, g_haship_pubkey_size > { 
	c_haship_pubkey(); 
	c_haship_pubkey( const std::string & text ); ///< create the IP form 
};

c_haship_pubkey::c_haship_pubkey() : std::array<unsigned char, g_haship_pubkey_size>({}) { }
c_haship_pubkey::c_haship_pubkey( const std::string & text ) : std::array<unsigned char, g_haship_pubkey_size>({}) { }

// ------------------------------------------------------------------

class c_tunserver {
	public:
		c_tunserver();

		void configure(const std::vector<std::string> & args); ///< load configuration
		void configure(int K, const std::string &mypub, const std::string &mypriv, const std::string &peerip, const std::string &peerpub);
		void run(); ///< run the main loop
		void configure_add_peer(const c_ip46_addr & addr_peering, const c_haship_pubkey & pubkey); ///< add this as peer
		void help_usage() const; ///< show help about usage of the program

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop
		void wait_for_fd_event(); ///< waits for event of I/O being ready, needs valid m_tun_fd and others, saves the fd_set into m_fd_set_data
		void print_destination_ipv6(const char *buff, size_t budd_size);

	private:
		int m_tun_fd; ///< fd of TUN file

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		typedef std::map< c_haship_addr, unique_ptr<c_peering> > t_peers_by_haship; ///< my peers (we always know their IPv6 - we assume here)
		t_peers_by_haship m_peer; ///< my peers 

		int m_myip_fill; ///< my test fill for the IP address generation
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_tunserver::c_tunserver()
 : m_tun_fd(-1), m_sock_udp(-1),
 m_myip_fill(1) // default IP
{
}

void c_tunserver::configure_add_peer(const c_ip46_addr & addr_peering, const c_haship_pubkey & pubkey) {
	_note("Adding peer, peering-address=" << addr_peering ); // @TODO << " pubkey=" << pubkey);
	_erro("DISABLED");
	// auto haship_addr = c_haship_addr( pubkey ); // XXX
	// m_peer.emplace( std::make_pair( haship_addr ,  make_unique<c_peering_udp>( addr_peering , pubkey , haship_addr ) ) ); // XXX
}

void c_tunserver::configure(const std::vector<std::string> & args) {
	bool valid=false;
	try {
		if (args.size()>=5+1+1) {
			{	const int i=1;
				if (args.at(i) == "-K") { // -K 5 mypub mypriv
					m_myip_fill = std::atoi( args.at(i+1).c_str() );
				}
			}

			{	const int i=5;
				if (args.at(i) == "-p") {
					configure_add_peer( c_ip46_addr::create_ipv4(args.at(i+1),9042) , args.at(i+2) );
				}
			}
			valid=true;
		}
	} catch(...) { valid=false; }

	if (!valid) {
		_erro("Invalid program options");
		help_usage();
		throw std::runtime_error("Fix program options");
	}
}

void c_tunserver::configure(int K, const string &mypub, const string &mypriv, const string &peerip, const string &peerpub) {
	m_myip_fill = K;
	if (c_ip46_addr::is_ipv4(peerip)) {
		configure_add_peer(c_ip46_addr::create_ipv4(peerip, 9042), peerpub);
	}
	else {
		configure_add_peer(c_ip46_addr::create_ipv6(peerip, 9042), peerpub);
	}
	// TODO
}


void c_tunserver::help_usage() const {
	std::ostream & out = cerr;
	out << "Usage:" << endl
		<< "program -K ipfill mypub mypriv -p peerip peerpub" << endl
		<< "program -K 5 mypub mypriv -p 192.168.0.5 peerpub" << endl
		<< "  ipfill - number that sets your virtual IP address for now, 0-255" << endl
		<< "  mypub - your public key (give any string, not yet used)" << endl
		<< "  mypriv - your PRIVATE key (give any string, not yet used - of course this is just for tests)" << endl
		<< "  peerip - IP over existing networking to connect to your peer" << endl
		<< "  peerpub - public key of your peer" << endl
	;
}

void c_tunserver::prepare_socket() {
	m_tun_fd = open("/dev/net/tun", O_RDWR);
	assert(! (m_tun_fd<0) );

  as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TAP; // || IFF_MULTI_QUEUE; TODO
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, (void *)&ifr);
	if (errcode_ioctl < 0)_throw( std::runtime_error("Error in ioctl")); // TODO

	_mark("Allocated interface:" << ifr.ifr_name);

	{
		uint8_t address[16];
		for (int i=0; i<16; ++i) address[i] = 0;
		// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
		// ...to the device to which we are setting IP address here:
		address[0] = 0xFD;
		address[1] = 0x42;
		address[2] = m_myip_fill;
		NetPlatform_addAddress(ifr.ifr_name, address, 8, Sockaddr_AF_INET6);
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

void c_tunserver::print_destination_ipv6(const char *buff, size_t buff_size) {
	char ipv6_str [INET6_ADDRSTRLEN];
	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + 12, ipv6_str, INET6_ADDRSTRLEN);
	_dbg1("src ipv6_str " << ipv6_str);
	memset(ipv6_str, 0, INET6_ADDRSTRLEN);
	inet_ntop(AF_INET6, buff + 28, ipv6_str, INET6_ADDRSTRLEN);
	_dbg1("dst ipv6_str " << ipv6_str);
}


void c_tunserver::event_loop() {
	_info("Entering the event loop");
	c_counter counter(2,true);
	c_counter counter_big(10,false);

	fd_set fd_set_data;

	const int buf_size=65536;
	char buf[buf_size];

	while (1) {
		wait_for_fd_event();

		if (FD_ISSET(m_tun_fd, &m_fd_set_data)) { // data incoming on TUN - send it out to peers
			auto size_read = read(m_tun_fd, buf, sizeof(buf)); // read data from TUN

			_info("TUN read " << size_read << " bytes: [" << string(buf,size_read)<<"]");
			try {
				_erro("SENDING DISABLED FOR NOW"); // XXX
	//			auto peer_udp = unique_cast_ptr<c_peering_udp>( m_peer.at(0));
//				print_destination_ipv6(buf, size_read);
//				peer_udp->send_data_udp(buf, size_read, m_sock_udp);
			} catch(...) {
				_warn("Can not send to peer."); // TODO more info (which peer, addr, number)
			}
		}
		else if (FD_ISSET(m_sock_udp, &m_fd_set_data)) { // data incoming on peer (UDP) - will route it or send to our TUN
			sockaddr_in6 from_addr_raw; // the address of sender, raw format
			socklen_t from_addr_raw_size; // the size of sender address

			from_addr_raw_size = sizeof(from_addr_raw); // IN/OUT parameter to recvfrom, sending it for IN to be the address "buffer" size
			auto size_read = recvfrom(m_sock_udp, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>( & from_addr_raw), & from_addr_raw_size);
			// ^- reinterpret allowed by linux specs (TODO)
			// sockaddr *src_addr, socklen_t *addrlen);

			_info("UDP read " << size_read << " bytes: [" << string(buf,size_read)<<"]");
			// ------------------------------------
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
				continue;
			}
			// ------------------------------------

			// reinterpret for debug
			_info("UDP received, sending to TUN:" << decrypted_buf_len << " bytes: [" << string( reinterpret_cast<char*>(decrypted_buf.get()), decrypted_buf_len)<<"]" );
			write(m_tun_fd, decrypted_buf.get(), decrypted_buf_len);

		}
		else _erro("No event selected?!"); // TODO throw

		int sent=0;
		counter.tick(sent, std::cout);
		counter_big.tick(sent, std::cout);
	}
}

void c_tunserver::run() {
	std::cout << "Starting tests" << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------

namespace developer_tests {

bool wip_galaxy_route_star(boost::program_options::variables_map & argm) {
	namespace po = boost::program_options;
	std::cerr << "Running in developer mode. " << std::endl;

	const int node_nr = argm["develnum"].as<int>();  assert( (node_nr>=1) && (node_nr<=254) );
	std::cerr << "Running in developer mode - as node_nr=" << node_nr << std::endl;
	// string peer_ip = string("192.168.") + std::to_string(node_nr) + string(".62");
	string peer_ip = string("192.168.") + std::to_string( node_nr==1 ? 2 : 1  ) + string(".62"); // each connect to node .1., except the node 1 that connects to .2.
	argm.insert(std::make_pair("K", po::variable_value( int(node_nr) , false )));
	argm.insert(std::make_pair("peerip", po::variable_value( peer_ip , false )));
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
			("K", po::value<int>()->required(), "number that sets your virtual IP address for now, 0-255")
			("mypub", po::value<std::string>()->default_value("") , "your public key (give any string, not yet used)")
			("mypriv", po::value<std::string>()->default_value(""), "your PRIVATE key (give any string, not yet used - of course this is just for tests)")
			("peerip", po::value<std::string>()->required(), "IP over existing networking to connect to your peer")
			("peerpub", po::value<std::string>()->default_value(""), "public key of your peer");

		po::variables_map argm;
		try {
			po::store(po::parse_command_line(argc, argv, desc), argm);
			cout << "devel" << endl;
			if (argm.count("devel")) {
				bool should_continue = run_mode_developer(argm);
				if (!should_continue) return 0;
			}
			// argm now can contain options added/modified by developer mode
			po::notify(argm);


			if (argm.count("help")) {
				std::cout << desc;
				return 0;
			}
			myserver.configure(argm["K"].as<int>(), argm["mypub"].as<std::string>(), argm["mypriv"].as<std::string>(), argm["peerip"].as<std::string>(), argm["peerpub"].as<std::string>());
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


