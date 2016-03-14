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


#include "counter.hpp"



// ------------------------------------------------------------------ XXX move me


// --- to library --- TODO ------------------------------------------

template <typename T> void memzero(const T & obj) {
	std::memset( (void*) & obj , 0 , sizeof(T) );
}

template <typename T>
class as_zerofill : public T {
	public:
		as_zerofill() {
			assert( sizeof(*this) == sizeof(T) ); // TODO move to static assert. sanity check. quote isostd
			void* baseptr = static_cast<void*>( dynamic_cast<T*>(this) );
			assert(baseptr == this); // TODO quote isostd
			memset( baseptr , 0 , sizeof(T) );
		}
		T& get() { return *this; }
};

// ------------------------------------------------------------------

void error(const std::string & msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

// ------------------------------------------------------------------

class c_speed_stats {
};

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
		std::string m_target_addr; ///< our target: IP address
		int m_target_port; ///< our target: IP (e.g. UDP) port number
		bool m_target_is_ipv6; ///< is the target's address an IPv6

		int m_blocksize; ///< size of the block to send

		int m_sock; ///< the network socket
		as_zerofill< sockaddr_in  > m_sockaddr4 ; ///< socket address (ipv4)
		as_zerofill< sockaddr_in6 > m_sockaddr6 ; ///< socket address (ipv6)
};

// ------------------------------------------------------------------

using namespace std; // XXX move to implementations, not to header-files later, if splitting cpp/hpp

c_ipbench::c_ipbench() {
	m_blocksize=0;
	memzero(m_sockaddr4);
	memzero(m_sockaddr6);
}

void c_ipbench::configure(const std::vector<std::string> & args) {
	m_target_addr = args.at(1);
	m_target_port = atoi(args.at(2).c_str());

	string protocol_name = args.at(3);
	if (protocol_name=="ipv6") m_target_is_ipv6=1;
	else if (protocol_name=="ipv4") m_target_is_ipv6=0;
	else error(string("Bad protocol name '") + protocol_name + "', use instead name: 'ipv6' or 'ipv4'");

	m_target_is_ipv6 = args.at(2).c_str();
	m_blocksize = atoi(args.at(4).c_str());
	assert(m_blocksize >= 1);
}

void c_ipbench::prepare_socket() {
	if (! m_target_is_ipv6) { // ipv4
		m_sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (m_sock<0) error("Can not create socket");
		m_sockaddr4.sin_family = AF_INET;
		m_sockaddr4.sin_port = htons( m_target_port );

		std::cout << "Resolving the address (ipv4)" << std::endl;
		auto host_data = gethostbyname2(m_target_addr.c_str(), AF_INET);
		{ // convert
			unsigned char *addr_as_bytes = reinterpret_cast<unsigned char*>(m_sockaddr4.sin_addr.s_addr);
			assert( sizeof(m_sockaddr4.sin_addr.s_addr) == host_data->h_length); // do both have same number of bytes
			assert( sizeof(m_sockaddr4.sin_addr.s_addr) == 4); // just a sanity check
			for (int i=0; i<host_data->h_length; ++i) addr_as_bytes[i] = host_data->h_addr[i]; // TODO
		}
	}
	else { // ipv6
		m_sock = socket(AF_INET6, SOCK_DGRAM, 0);
		if (m_sock<0) error("Can not create socket");
		m_sockaddr6.sin6_family = AF_INET6;
		m_sockaddr6.sin6_port = htons( m_target_port );

		std::cout << "Resolving the address '" << m_target_addr << "' as ipv6..." << std::endl;
		auto host_data = gethostbyname2(m_target_addr.c_str(), AF_INET6);
		for (int i=0; i<host_data->h_length; ++i) m_sockaddr6.sin6_addr.s6_addr[i] = host_data->h_addr[i];
		std::cout << "Resolved." << std::endl;
	}

}

void c_ipbench::event_loop() {
	vector<char> buffer(m_blocksize,'a');
	c_counter counter(2,true);
	c_counter counter_big(10,false);
	while (1) {
		sockaddr_in6 * sockaddr6_ptr = & m_sockaddr6.get();
		sockaddr * addr_ptr = reinterpret_cast<sockaddr*>(sockaddr6_ptr); // guaranteed by Linux. TODO
		auto sent = sendto(m_sock, static_cast<void*>(buffer.data()), buffer.size(),  0,
			addr_ptr,	sizeof(m_sockaddr6));
		if (sent<0) error("Sent failed");

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


