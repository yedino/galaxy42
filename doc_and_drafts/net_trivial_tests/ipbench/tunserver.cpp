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

#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <string.h>
#include <assert.h>

#include <thread>

#include <cstring>

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
// #include <net/if_ether.h> // peer over eth later?
// #include <net/if_media.h> // ?

#include "../NetPlatform.h" // from cjdns

// #include <net/if_tap.h>
#include <linux/if_tun.h>

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------

// TODO to lib "ip46"

// sockaddr is the base class for sockaddr_in and for sockaddr_in6
// see also http://stackoverflow.com/a/18579605
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in) , "Invalid size of ipv4 vs ipv6 addresses" );
static_assert( sizeof(sockaddr) <= sizeof(sockaddr_in6) , "Invalid size of ipv4 vs ipv6 addresses" );

class c_ip46_addr { ///< any address ipv6 or ipv4, in system socket format
	public:
		typedef enum { tag_none, tag_ipv4, tag_ipv6 } t_tag; ///< possible address type

		c_ip46_addr();

		void set_ip4(sockaddr_in in4);
		void set_ip6(sockaddr_in6 in6);
		sockaddr_in  get_ip4() const;
		sockaddr_in6 get_ip6() const;

		t_tag get_ip_type() const;

		static c_ip46_addr any_on_port(int port); ///< return my address, any IP (e.g. for listening), on given port. it should listen on both ipv4 and 6
		static c_ip46_addr create_ipv4(const std::string &ipv4_str, int port);
		friend ostream &operator << (ostream &out, const c_ip46_addr& addr);

	private:
		struct t_ip_data {
			union { ///< the address is either:
				sockaddr_in in4;
				sockaddr_in6 in6;
			};
		};

		t_ip_data m_ip_data;

		t_tag m_tag; ///< current type of address
};

c_ip46_addr::c_ip46_addr() : m_tag(tag_none) { }

c_ip46_addr::t_tag c_ip46_addr::get_ip_type() const {
	return m_tag;
}

void c_ip46_addr::set_ip4(sockaddr_in in4) {
	_assert(in4.sin_family == AF_INET);
	m_tag = tag_ipv4;
	this->m_ip_data.in4 = in4;
}
void c_ip46_addr::set_ip6(sockaddr_in6 in6) {
	_assert(in6.sin6_family == AF_INET6);
	m_tag = tag_ipv6;
	this->m_ip_data.in6 = in6;
}

sockaddr_in  c_ip46_addr::get_ip4() const {
	_assert(m_tag == tag_ipv4);
	auto ret = this->m_ip_data.in4;
	_assert(ret.sin_family == AF_INET);
	return ret;
}
sockaddr_in6 c_ip46_addr::get_ip6() const {
	_assert(m_tag == tag_ipv6);
	auto ret = this->m_ip_data.in6;
	_assert(ret.sin6_family == AF_INET6);
	return ret;
}

c_ip46_addr c_ip46_addr::any_on_port(int port) { ///< return my address, any IP (e.g. for listening), on given port
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(port);
	addr_in.sin_addr.s_addr = INADDR_ANY;
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}

c_ip46_addr c_ip46_addr::create_ipv4(const string &ipv4_str, int port) {
	as_zerofill< sockaddr_in > addr_in;
	addr_in.sin_family = AF_INET;
	inet_pton(AF_INET, ipv4_str.c_str(), &(addr_in.sin_addr));
	addr_in.sin_port = htons(port);
	c_ip46_addr ret;
	ret.set_ip4(addr_in);
	return ret;
}


ostream &operator << (ostream &out, const c_ip46_addr& addr) {
	if (addr.m_tag == c_ip46_addr::tag_ipv4) {
		char addr_str[INET_ADDRSTRLEN];
		auto ip4_address = addr.get_ip4();
		inet_ntop(AF_INET, &ip4_address.sin_addr, addr_str, INET_ADDRSTRLEN);
		out << addr_str;
	}
	else if (addr.m_tag == c_ip46_addr::tag_ipv6) {
		char addr_str[INET6_ADDRSTRLEN];
		auto ip6_address = addr.get_ip6();
		inet_ntop(AF_INET6, &ip6_address.sin6_addr, addr_str, INET6_ADDRSTRLEN);
		out << addr_str;
	}
	else {
		out << "none";
	}
	return out;
}

// ------------------------------------------------------------------

// TODO: crypto options here
class c_peering { ///< An (mostly established) connection to peer
	public:
		c_peering(const c_ip46_addr & addr, const std::string & pubkey);

		virtual void send_data(const char * data, size_t data_size)=0;
		virtual ~c_peering()=default;

	protected:
		c_ip46_addr	m_addr; ///< peer address in socket format

		std::string m_pubkey; ///< his pubkey
		// ... TODO crypto type
};

c_peering::c_peering(const c_ip46_addr & addr, const std::string & pubkey)
 : m_addr(addr), m_pubkey(pubkey)
{
	_info("I am new peer, with addr="<<addr<<" and pubkey="<<pubkey);
}

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
		c_peering_udp(const c_ip46_addr & addr, const std::string & pubkey);

		virtual void send_data(const char * data, size_t data_size);
		virtual void send_data_udp(const char * data, size_t data_size, int udp_socket);
	private:
};

c_peering_udp::c_peering_udp(const c_ip46_addr & addr, const std::string & pubkey)
	: c_peering(addr, pubkey)
{ }

void c_peering_udp::send_data(const char * data, size_t data_size) {
	throw std::runtime_error("Use send_data_udp");
}

void c_peering_udp::send_data_udp(const char * data, size_t data_size, int udp_socket) {
	// TODO encrpt

	_info("UDP send to peer: " << data_size << " bytes: [" << string(data,data_size)<<"]" );

	switch (m_addr.get_ip_type()) {
		case c_ip46_addr::t_tag::tag_ipv4 : {
			auto ip_x = m_addr.get_ip4(); // ip of proper type, as local variable
			sendto(udp_socket, data, data_size, 0, reinterpret_cast<sockaddr*>( & ip_x ) , sizeof(sockaddr_in) );
		}
		break;
		case c_ip46_addr::t_tag::tag_ipv6 : {
			auto ip_x = m_addr.get_ip6(); // ip of proper type, as local variable
			sendto(udp_socket, data, data_size, 0, reinterpret_cast<sockaddr*>( & ip_x ) , sizeof(sockaddr_in6) );
		}
		break;
		default: {
			std::ostringstream oss; oss << m_addr; // TODO
			throw std::runtime_error(string("Invalid IP type: ") + oss.str());
		}
	}
}

// ------------------------------------------------------------------

class c_tunserver {
	public:
		c_tunserver();

		void configure(const std::vector<std::string> & args); ///< load configuration
		void run(); ///< run the main loop
		void configure_add_peer(const c_ip46_addr & addr, const std::string & pubkey); ///< add this as peer
		void help_usage() const; ///< show help about usage of the program

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop
		void wait_for_fd_event(); ///< waits for event of I/O being ready, needs valid m_tun_fd and others, saves the fd_set into m_fd_set_data

	private:
		int m_tun_fd; ///< fd of TUN file

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)

		fd_set m_fd_set_data; ///< select events e.g. wait for UDP peering or TUN input

		vector<unique_ptr<c_peering>> m_peer; ///< my peers

		int m_myip_fill; ///< my test fill for the IP address generation
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_tunserver::c_tunserver()
 : m_tun_fd(-1), m_sock_udp(-1),
 m_myip_fill(1) // default IP
{
}

void c_tunserver::configure_add_peer(const c_ip46_addr & addr, const std::string & pubkey) {
	_note("Adding peer, address=" << addr << " pubkey=" << pubkey);
	m_peer.push_back( make_unique<c_peering_udp>( addr , pubkey ) );
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
	ifr.ifr_flags = IFF_TAP || IFF_MULTI_QUEUE;
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
		sockaddr_in addr4 = address_for_sock.get_ip4();
		auto bind_result = bind(m_sock_udp, reinterpret_cast<sockaddr*>(&addr4), sizeof(addr4));  // reinterpret allowed by Linux specs
		_assert( bind_result >= 0 ); // TODO change to except
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
				auto peer_udp = unique_cast_ptr<c_peering_udp>( m_peer.at(0));
				peer_udp->send_data_udp(buf, size_read, m_sock_udp);
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
			// decrypt !!! TODO

			_info("UDP received, sending to TUN:" << size_read << " bytes: [" << string(buf,size_read)<<"]" );
			write(m_tun_fd, buf, size_read);

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
	vector <string> args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	myserver.configure(args);
	myserver.run();
}


