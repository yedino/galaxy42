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
	enum { tag_none, tag_ipv4, tag_ipv6 } m_tag; ///< current type of address

	struct t_ip_data {
		union { ///< the address is either:
			sockaddr_in in4;
			sockaddr_in6 in6;
		};
	};

	t_ip_data m_ip_data;

	c_ip46_addr();

	void set_ip4(sockaddr_in in4);
	void set_ip6(sockaddr_in6 in6);
	sockaddr_in  get_ip4() const;
	sockaddr_in6 get_ip6() const;
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

// ------------------------------------------------------------------

// TODO: crypto options here
class c_peering { ///< An (mostly established) connection to peer
	public:
	private:
		c_ip46_addr	m_addr; ///< peer address in socket format
};

class c_peering_udp : public c_peering { ///< An established connection to UDP peer
	public:
	//	c_peering_udp(bool 
	private:
		// TODO optimize memory waste (union? or casting?)
		bool m_sockaddr_is6; ///< is it ipv6?
		as_zerofill< sockaddr_in  > m_sockaddr4 ; ///< socket address (ipv4)
		as_zerofill< sockaddr_in6 > m_sockaddr6 ; ///< socket address (ipv6)
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

		int m_sock_udp; ///< the main network socket (UDP listen, send UDP to each peer)
		as_zerofill< sockaddr_in  > m_udp_sockaddr4 ; ///< socket address (ipv4) for m_sock_udp
		as_zerofill< sockaddr_in6 > m_sockaddr6 ; ///< socket address (ipv6) for m_sock_udp

		int m_tun_fd; ///< fd of TUN file

		std::string m_target_addr; ///< our target: IP address XXX
		int m_target_port; ///< our target: IP (e.g. UDP) port number XXX
		bool m_target_is_ipv6; ///< is the target's address an IPv6 XXX

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

	std::cerr << sizeof(sockaddr_in) << " " << sizeof(sockaddr_in6)  << std::endl;

	std::cerr << disclaimer << std::endl;
	c_tunserver bench;
	vector <string> args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	bench.configure(args);
	bench.run();
}


