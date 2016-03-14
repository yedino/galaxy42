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

// #include <net/if_tap.h>
#include <linux/if_tun.h>

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------

class c_ipbench {
	public:
		c_ipbench();

		void configure(const std::vector<std::string> & args);
		void run();

	protected:
		void prepare_socket(); ///< make sure that the lower level members of handling the socket are ready to run
		void event_loop(); ///< the main loop

	private: 

		int m_tun_fd; ///< fd of TUN file

		std::string m_target_addr; ///< our target: IP address XXX
		int m_target_port; ///< our target: IP (e.g. UDP) port number XXX
		bool m_target_is_ipv6; ///< is the target's address an IPv6 XXX

		int m_sock; ///< the network socket XXX
		as_zerofill< sockaddr_in  > m_sockaddr4 ; ///< socket address (ipv4) XXX
		as_zerofill< sockaddr_in6 > m_sockaddr6 ; ///< socket address (ipv6) XXX
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_ipbench::c_ipbench() {
}

void c_ipbench::configure(const std::vector<std::string> & args) {
}

void c_ipbench::prepare_socket() {
	m_tun_fd = open("/dev/net/tun", O_RDWR);
	assert(! (m_tun_fd<0) );

  as_zerofill< ifreq > ifr; // the if request
	ifr.ifr_flags = IFF_TAP || IFF_MULTI_QUEUE;
	strncpy(ifr.ifr_name, "galaxy%d", IFNAMSIZ);
	auto errcode_ioctl =  ioctl(m_tun_fd, TUNSETIFF, (void *)&ifr);
	_throw( std::runtime_error("Error in ioctl")); // TODO
}

void c_ipbench::event_loop() {
	c_counter counter(2,true);
	c_counter counter_big(10,false);
	while (1) {
		int sent=0;
		counter.tick(sent, std::cout);
		counter_big.tick(sent, std::cout);
	}
}

void c_ipbench::run() {
	std::cout << "Starting tests" << std::endl;
	prepare_socket();
	event_loop();
}

// ------------------------------------------------------------------


int main(int argc, char **argv) {
	std::cerr << disclaimer << std::endl;
	c_ipbench bench;
	vector <string> args;
	for (int i=0; i<argc; ++i) args.push_back(argv[i]);
	bench.configure(args);
	bench.run();
}


