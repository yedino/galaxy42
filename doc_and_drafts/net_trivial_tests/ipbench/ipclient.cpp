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


// ------------------------------------------------------------------ XXX move me

class c_counter {
	public:
		typedef long long int t_count;

		c_counter(c_counter::t_count tick_len, bool is_main); ///< tick_len - how often should we fire (print stats, and restart window)

		void add(c_counter::t_count bytes); ///< general type for integrals (number of packets, of bytes)
		bool tick(c_counter::t_count bytes, std::ostream &out); ///< a tick, can look at clock (in reasonable way), can call print(out); returns: should we print new line
		void print(std::ostream &out) const; ///< prints now the statistics (better instead call tick)

	private:
		const t_count m_tick_len; ///< how often should I tick - it's both the window size, and the rate of e.g. print()
		bool m_is_main; ///< is this the main counter (then show global stats and so on)

		t_count m_pck_all, m_pck_w; ///< packets count: all, and in current window
		t_count m_bytes_all, m_bytes_w; ///< the bytes (in current windoow)
		t_count m_time_first; ///< when I was started at first actually (time in unix time)
		t_count m_time_ws; ///< when when current window started (time in unix time)
		t_count m_time_last; ///< current last time

		bool tick_restarts;
};

c_counter::c_counter(c_counter::t_count tick_len, bool is_main)
	: m_tick_len(tick_len), m_is_main(is_main),
	m_pck_all(0), m_pck_w(0), m_bytes_all(0), m_bytes_w(0)
{
	m_time_first=time(NULL);
	m_time_ws=time(NULL);
}

void c_counter::add(c_counter::t_count bytes) { ///< general type for integrals (number of packets, of bytes)
	m_pck_all += 1;
	m_pck_w += 1;

	m_bytes_all += bytes;
	m_bytes_w += bytes;
}

bool c_counter::tick(c_counter::t_count bytes, std::ostream &out) { ///< a tick, can look at clock (in reasonable way), can call print(out)
	add(bytes);

	bool do_print=0;
	bool do_reset=0;
	if (m_pck_all==1) { do_print=1; do_reset=1; }
	if (0 == (m_pck_all%1000)) {
		// std::cerr<<m_bytes_all<<std::endl; std::cout << m_time_last << " >? " << m_time_ws << std::endl;
		m_time_last = std::time(NULL);
		if (m_time_last >= m_time_ws + m_tick_len) { do_reset=1; do_print=1; }
	}
	if (do_print)	print(out);
	if (do_reset) {
		m_time_last = std::time(NULL);
		m_time_ws = m_time_last;
		m_pck_w=0;
		m_bytes_w=0;
	}
	return do_print;
}

void c_counter::print(std::ostream &out) const { ///< prints now the statistics (better instead call tick)
	using std::setw;

	double time_all = m_time_last - m_time_first;
	double time_w = m_time_last - m_time_ws;
	const double epsilon = 1;
	time_all = std::max(epsilon, time_all);
	time_w   = std::max(epsilon, time_w);

	double avg_bytes_all = m_bytes_all / time_all;
	double avg_pck_all   = m_pck_all   / time_all;
	double avg_bytes_w = m_bytes_w / time_w;
	double avg_pck_w   = m_pck_w   / time_w;

	// formatting
	int p1=3; // digits after dot
	int w1=p1+1+5; // width

	double K=1000, Mi = 1024*1024, Gi = 1024*Mi; // units
	out	<< std::setprecision(3) << std::fixed;

	if (m_is_main) {
		out << setw(6) << m_bytes_all/Gi << "GiB; "
		    << "Speed: "
	             << setw(w1) << (avg_pck_all / K) << " Kpck/s "
		    << ",  " << setw(w1) << (avg_bytes_all*8 / Mi) << " Mib/s "
		    << " = " << setw(w1) << (avg_bytes_all   / Mi) << " MiB/s " << "; "
		    ;
	}
	out << "Window " << time_w << "s: "
	             << setw(w1) << (avg_pck_w   / 1000) << " Kpck/s "
	    << ",  " << setw(w1) << (avg_bytes_w  *8 / Mi) << " Mib/s "
	    << " = " << setw(w1) << (avg_bytes_w     / Mi) << " MiB/s " << ") ";
	out << std::endl;
}



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


