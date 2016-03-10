/**
See LICENCE.txt
*/

#include <iostream>
#include <stdexcept>


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


void error(const char *msg) {
	std::cout << "Error: " << msg << std::endl;
	throw std::runtime_error(msg);
}

class cIpBench {
	public:
		cIpBench();

		void Configure(const & std::vector<std::string> args);
		void Run();

	private:
		std::string m_target_addr; ///< our target: IP address
		int m_target_port; ///< our target: IP (e.g. UDP) port number
		bool m_target_is_ipv6; ///< is the target's address an IPv6

		int m_blocksize; ///< size of the block to send
};

cIpBench::cIpBench() {
	m_blocksize=0;
}

void cIpBench::Configure(const & std::vector<std::string> args) {
	m_target_addr = args.at(1);
	m_target_port = atoi(args.at(2));
	m_target_is_ipv6 = atoi(args.at(2));
	m_blocksize = atoi(args.at(4));
}

void cIpBench::Run() {

}


int main(int argc, char **argv) {
	cIpBench bench;
	bench.Run(argc,argv);
}


