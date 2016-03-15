/**
See LICENCE.txt
*/

const char * disclaimer = "*** WARNING: This is a work in progress, do NOT use this code, it has bugs, vulns, and 'typpos' everywhere. ***"; // XXX

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
		enum { tag_none, tag_ipv4, tag_ipv6 } m_tag; ///< current type of address

	public:
		c_ip46_addr();

		void set_ip4(sockaddr_in in4);
		void set_ip6(sockaddr_in6 in6);
		sockaddr_in  get_ip4() const;
		sockaddr_in6 get_ip6() const;

		static c_ip46_addr any_on_port(int port); ///< return my address, any IP (e.g. for listening), on given port. it should listen on both ipv4 and 6
		friend ostream &operator << (ostream &out, const c_ip46_addr& addr);

	private:
		struct t_ip_data {
			union { ///< the address is either:
				sockaddr_in in4;
				sockaddr_in6 in6;
			};
		};

		t_ip_data m_ip_data;
};

c_ip46_addr::c_ip46_addr() : m_tag(tag_none) { }

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
	private:
		c_ip46_addr	m_addr; ///< peer address in socket format
};

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
	private:
};

// ------------------------------------------------------------------

class c_tunserver {
	public:
		c_tunserver();

		void configure(const std::vector<std::string> & args);
		void run();

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop

	private: 
		int m_tun_fd; ///< fd of TUN file

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_tunserver::c_tunserver() {
}

void c_tunserver::configure(const std::vector<std::string> & args) {
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

	uint8_t address[16];
	for (int i=0; i<16; ++i) address[i] = 8;

	// TODO: check if there is no race condition / correct ownership of the tun, that the m_tun_fd opened above is...
	// ...to the device to which we are setting IP address here:
	address[0] = 0xFD;
	address[1] = 0x00;
	NetPlatform_addAddress(ifr.ifr_name, address, 8, Sockaddr_AF_INET6);

	// create listening socket
	m_sock_udp = socket(AF_INET, SOCK_DGRAM, 0);
	_assert(m_sock_udp >= 0);

	int port = 9042;
	c_ip46_addr address_for_sock = c_ip46_addr::any_on_port(port);

	{
		auto addr4 = address_for_sock.get_ip4();
		//auto bind_result = bind(sock, static_cast<sockaddr*>(&addr4), sizeof(addr4));
		//_assert( bind_result >= 0 ); // TODO change to except
	}
	_info("Bind done - listening on UDP on: "); // TODO  << address_for_sock
}

void c_tunserver::event_loop() {
	c_counter counter(2,true);
	c_counter counter_big(10,false);
	while (1) {
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
	std::cerr << disclaimer << std::endl;

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


