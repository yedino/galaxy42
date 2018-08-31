#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <boost/asio.hpp>
#include <chrono>
#include <limits>
#include <thread>
#include <functional>

#define pfp_warn(X) { std::cerr << __LINE__ << " WARNING: " << X << std::endl; }
#define pfp_info(X) { std::cerr << __LINE__ << " Info: " << X << std::endl; }
#define pfp_dbg1(X) { std::cerr << __LINE__ << " Dbg : " << X << std::endl; }


///////////////////////////////////////////////////////////////////////////////////////////
class c_rpc final {
	public:
		c_rpc(std::function<void(const std::string &)> f, unsigned int timeout_minutes);
		void start_listen(boost::asio::ip::address_v4 listen_address, unsigned short port, boost::asio::ip::address_v4 rpc_authorized_ip);
	private:
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::socket m_socket;
		std::function<void(const std::string &)>m_rpc_fun;
		std::string m_input_buffer;
		std::chrono::steady_clock::time_point m_stop_point;

		bool rpc_time_limit_hit() const;
};

c_rpc::c_rpc(std::function<void(const std::string &)> f, unsigned int timeout_minutes)
:
	m_io_service(),
	m_socket(m_io_service),
	m_rpc_fun(f),
	m_input_buffer(),
	m_stop_point(std::chrono::steady_clock::now() + std::chrono::minutes(timeout_minutes))
{
	pfp_info("Starting RPC server with max timeout_minutes=" << timeout_minutes);
}

bool c_rpc::rpc_time_limit_hit() const {
	if ( std::chrono::steady_clock::now() >= m_stop_point ) return true;
	return false;
}

void c_rpc::start_listen(boost::asio::ip::address_v4 listen_address, unsigned short port, boost::asio::ip::address_v4 rpc_authorized_ip) {
	boost::asio::ip::tcp::acceptor acceptor(m_io_service, boost::asio::ip::tcp::endpoint(listen_address, port));

	struct c_raii_guard { std::function<void()> f; ~c_raii_guard() { f(); } }; // TODO move to stdplus lib?

	while (true) {
		std::cout << "\n\n\n";
		pfp_info("Will wait for connection on " << listen_address << ":" << port);

		if (this->rpc_time_limit_hit()) { pfp_info("RPC time limit hit!");  return; }
		acceptor.accept(m_socket); // blocks
		c_raii_guard raii_socket{ [this](){ this->m_socket.close(); } };

		if (this->rpc_time_limit_hit()) { pfp_info("RPC time limit hit!");  return; }

		boost::asio::ip::tcp::endpoint local_endpoint, remote_endpoint;
		local_endpoint = m_socket.local_endpoint();
		remote_endpoint = m_socket.remote_endpoint();

		pfp_info("Got connection to RPC, from: " << remote_endpoint << " to our local endpoint " << local_endpoint);

		if ( rpc_authorized_ip != boost::asio::ip::address_v4::any() ) {
			if (rpc_authorized_ip != remote_endpoint.address()) {
				pfp_warn("IP is not authorized! Allowed is: " << rpc_authorized_ip << " instead of " << remote_endpoint);
				continue ; // <--- !
			}
		}

		/*
		// TODO this doesn't work well, especially breaks on localhost (that needs to listen on 0.0.0.0)
		// automatic mask for given IP class?
		boost::asio::ip::address_v4 local_mask = boost::asio::ip::address_v4::netmask(local_endpoint.address().to_v4());
		boost::asio::ip::address_v4 remote_mask = boost::asio::ip::address_v4::netmask(remote_endpoint.address().to_v4());
		if (local_mask != remote_mask) {
			throw std::runtime_error("Bad remote address netmask, local: " + local_mask.to_string() + " remote " + remote_mask.to_string());
		}
		*/

		const std::string ok_message = "DONE\n";
		try {
			while (true) { // for each command in one TCP connection
				if (this->rpc_time_limit_hit()) { pfp_info("RPC time limit hit!");  return; }
				std::cout << "\n\n";
				pfp_info("RPC command read...");
				boost::asio::streambuf input_stream;
				boost::asio::read_until(m_socket, input_stream, '\n');
				std::istream istream(&input_stream);
				std::getline(istream, m_input_buffer);
				pfp_info("RPC command read...: [" << m_input_buffer << "]");
				boost::asio::write(m_socket, boost::asio::buffer("START\n"));

				m_rpc_fun(m_input_buffer); // ***

				boost::asio::write(m_socket, boost::asio::buffer(ok_message));
			}
		} catch(const std::exception & ex) {
			pfp_info("RPC session/TCP ended with exception: " << ex.what());
		}

	}
}
///////////////////////////////////////////////////////////////////////////////////////////

using boost::asio::ip::udp;
using std::endl;

enum { max_length = 655035 };

using mysize_t = uint64_t; // size_t is too small to count some things, and overflows, e.g. on r-pi

template <class TOut>
void explode(const std::string & text, char sep, TOut & output) {
    std::istringstream iss(text);
    std::string word;
    while (std::getline(iss, word, sep)) output.push_back(std::move(word));
}

std::vector<std::string> explode(const std::string & text, char sep) {
	std::vector<std::string> ret;
	explode(text,sep,ret);
	return ret;
}

std::string remove_extra_whitespace(const std::string  & str) {
	std::string ret;
	bool last_was = false; // was whitespace
	const auto len = str.size();
	for (size_t i=0; i<len; ++i) {
		char c = str[i];
		bool now_is = (c == ' '); // now it is a whitespace?:w
		if ( ! (last_was && now_is) ) ret += c;
		last_was = now_is;
	}
	return ret;
}

class c_maintask {
	public:
		c_maintask();

		int run(int argc, const char * argv []);
		int run_remote(int argc, const char * argv []);

		std::string run_rpc_command_string(const std::string & rpc_cmd);

		void print_usage() const;
		void print_help_sendcommand() const;
		void print_usage_rpc_cmd() const;

	private:

		// asio var:
		boost::asio::io_service io_service;
		udp::socket mysocket;
		udp::resolver resolver;
		udp::endpoint endpoint;

		// buffers:
		char request[max_length];
		mysize_t burst;

		// other var:
		mysize_t counter_sleep_no, counter_sleep_yes;
		mysize_t packets_sent, size_sent; // size is in [Bytes]

		std::chrono::microseconds conf_print_interval; // how often should we print stats
		std::chrono::steady_clock::time_point time_start;
		std::chrono::steady_clock::time_point time_print_next;

		double target_speed_Mbit_sec;
		double target_speed_MByte_microsec;

		// conf:
		const char *host;
		const char *port;
		mysize_t speed; ///< Mbps
		std::string message;
		bool interactive;
		mysize_t bytes, count, request_length;
		bool count_infinite; // infinite count sends forever

		std::string m_rpc_password;

	//	udp::socket m_remote_sock; ///< local socket to receive remote commands (in remote mode)
};

c_maintask::c_maintask()
: mysocket(io_service), resolver(io_service)
{
	m_rpc_password="";
}

void c_maintask::print_help_sendcommand() const {
	std::cout << "SendCommand: " << endl
		<< "  foo 0 1    - this will send text foo (1 time)" << std::endl
		<< "  abc 50 1000   - this will send message of letter 'a' repeated 50 times. This msg will be sent 1000 times."
		<<std::endl;
}

void c_maintask::print_usage_rpc_cmd() const {
	std::cout << "secret1 SEND [args for normal run]  <--- this text, ending with a NEW LINE.\n";
	std::cout << "secret1 SEND 127.0.0.1 5555  250000 foo 1472  -1  \n";
	std::cout << "secret1 SEND 127.0.0.1 5555  250000 foo 8972  -1 5 -1 (no time limit, 5 seconds, no GB limit) \n";
}

void c_maintask::print_usage() const {
		std::cout << std::endl;
		std::cout << "Usage: ./client <host> <port> <max_speed Mbps> SendCommand" << std::endl;
		std::cout << "e.g.:  ./client <host> <port> <max_speed Mbps> <msg> <msgbytes> <count> [maxTime sec] [maxData GiB]" << std::endl;
		std::cout << "e.g.:  ./client 127.0.0.1 5555  250000    foo   1472       -1 " << std::endl;
		std::cout << "e.g.:  ./client 127.0.0.1 5555  250000    foo   1472       -1 10 -1 " << std::endl;
		std::cout << "\nor start server accepting remote RPC commands: \n";
		std::cout << "e.g.:  ./client remote_cmd 192.168.70.17 5555 " << std::endl;
		std::cout << "or instead allow remote access (remote controll of this sending, WARNING can make your computer spam/DDoS other computer) " << std::endl;
		std::cout << "e.g.:  ./client remote <listen_on_ip><port>  <authorized_ip> <max_time_minutes> <pass>  " << std::endl;
		std::cout << "e.g.:  ./client remote 192.168.70.17 5011   192.168.70.16   72 secret1" << std::endl;
		std::cout << "e.g.:  ./client remote 192.168.70.17 5011   0.0.0.0         4  secret1" << std::endl;
		std::cout << "e.g.:  ./client remote       0.0.0.0 5011   0.0.0.0         4  secret1" << std::endl;
		std::cout << "then send TCP text like e.g.: \n";
		print_usage_rpc_cmd();
		std::cout << std::endl;
}

std::string c_maintask::run_rpc_command_string(const std::string & rpc_cmd) {
	std::string ret;
	ret += "RPC-result ";
	std::cout << "RPC command to execute: [" << rpc_cmd << "]\n" << std::endl;
	std::vector<std::string> args = explode( remove_extra_whitespace(rpc_cmd) , ' ');
	std::cout << "RPC command args size: " << args.size() << std::endl;

	try {
		std::string password_given = args.at(0);
		std::string word_SEND = args.at(1);

		if (m_rpc_password.size()<3) throw std::runtime_error("insecure (short) or missing RPC password (configured here)");
		if (password_given != m_rpc_password) throw std::runtime_error("invalid RPC password given");
		if (word_SEND != "SEND") throw std::runtime_error("word 'SEND'");

		args.erase( args.begin() , args.begin()+2 );

	} catch(const std::exception &ex) {
		pfp_warn("Can not parse RPC: " << ex.what() << " rpc was [" << rpc_cmd << "]");
		throw ;
	}

	args.insert( args.begin(), "program_name" );

	size_t argc = args.size();
	std::vector<const char*> argv( argc , nullptr );
	for (size_t i=0; i<argc; ++i) argv[i] = args.at(i).c_str(); // ! points to memory owned by strings in vector args!
	for (size_t i=0; i<argc; ++i) pfp_info("argv["<<i<<"] = [" << argv[i] << "]");
	std::string error_msg="(no error)";
	try {

		c_maintask maintask_once; // fresh new program (to not reuse our object this)
		maintask_once.run( argc, & argv.at(0) );

	} catch (const std::exception & ex) {
		error_msg = ex.what();
		pfp_warn("Exception while tryint to execute RPC command: " << error_msg);
	}

	std::cout << "RPC command DONE      : [" << rpc_cmd << "]" << std::endl;
	return ret;
}

int c_maintask::run_remote(int argc, const char * argv[]) {
	pfp_info("Starting remote server");
	if (argc < 7) { print_usage(); return 1; }

	this->m_rpc_password = argv[6];
	pfp_info("RPC password has length: " << this->m_rpc_password.size());

	c_rpc rpc([this](const std::string &rpc_data) {
			pfp_info("rpc data [" << rpc_data << "]\n\n");
			try {
				run_rpc_command_string(rpc_data);
			} catch(const std::exception &ex) {
				pfp_info("RPC request failed [" << ex.what() << ")");
			}
		},
		std::stoi(argv[5]) // timeout
	);

	boost::asio::ip::address_v4 rpc_listen_address = boost::asio::ip::address_v4::from_string(argv[2]);
	unsigned short rpc_port = std::stoi(argv[3]);
	boost::asio::ip::address_v4 rpc_authorized_ip = boost::asio::ip::address_v4::from_string(argv[4]);
	pfp_info( "any : " << boost::asio::ip::address_v4::any() );
	pfp_info("RPC will listen on: " << rpc_listen_address << ":" << rpc_port
		<< " and will allow connections from: "
		<< rpc_authorized_ip << (rpc_authorized_ip==boost::asio::ip::address_v4::any() ? "(any)" : ""));
	pfp_info("You can use following RPC commands:");
	this->print_usage_rpc_cmd();
	rpc.start_listen(rpc_listen_address, rpc_port, rpc_authorized_ip); // blocks
	return 0;
}

int c_maintask::run(int argc, const char * argv[])
{
	pfp_info("Starting the sending run");

	if (argc < 3) {
		pfp_warn("Too few options");
		print_usage();
		print_help_sendcommand();
		return 1;
	}
	pfp_dbg1("Parsing (start)");
	host = argv[1];
	port = argv[2];
	speed = std::stoi(argv[3]);
	burst = 50;

	interactive=true;
	count_infinite=false; // infinite count sends forever
	pfp_dbg1("Parsing (after first)");

	if (argc >= 2+3+1) {
		pfp_dbg1("Parsing (long)");
		interactive=false;
		message = argv[4];
		bytes = std::stoi(argv[5]);
		double count_exact = std::stod(argv[6]);
		if (count_exact < 0) {
			count=0; count_infinite=true;
		}
		else {
			count = static_cast<decltype(count)>(count_exact);
			count_infinite=false;
		}
	}

	double limit_data_GiB = -1 ; // -1 means infinite
	double limit_time_sec = -1 ; // -1 means infinite

	if (argc > 7) limit_time_sec = atof(argv[7]);
	if (argc > 8) limit_data_GiB  = atof(argv[8]);

	std::cerr << "Limit time: " << limit_time_sec << " sec (since start)" << std::endl;

	mysocket.open( udp::v4() );
	endpoint = * resolver.resolve({udp::v4(), host, port});

	std::cout << std::endl;

	counter_sleep_no=0;
	counter_sleep_yes=0;

	// the main loop, mainly used for interactive mode
	while(true) {
		if (interactive) {
			std::cout << "\n";
			print_help_sendcommand();
			std::cout << "Enter: message bytes count";
			std::cout << std::endl << "-> ";
			std::cin >> message >> bytes >> count;
			std::cout<<"\n";
		}
		if (message.size() > max_length) {
			std::cout << "too long message" << std::endl;
			continue;
		}

		if (bytes == 0) {
			request_length = message.size();
			std::cout << "Using length: " << request_length << std::endl;
			assert(request_length < max_length-1);
			std::strncpy( request , message.c_str() , request_length);
		}
		else {
			request_length = bytes;
			assert(message.size()>=1);
			char ch = message.at(0);
			std::fill_n(request, request_length, ch);
		}

		std::cerr << "Sending now ";
		if (count_infinite) std::cerr << " infinite "; else std::cerr << count;
		std::cerr << " time(s) an UDP packet to [" << host << " port "<<port<<"]: ";
		std::cerr << "datagram: size " << request_length << " B " << "\n";
		// std::cerr << "request=\"" << request << "\"" << "\n";
		std::cerr << "message=\"" << message << "\"" << "\n";
		std::cerr << "data begins with \""
			<< std::string(request, request + std::min(static_cast<size_t>(10),request_length))
			<< "\"" << std::endl;
		std::cerr << endl;

		conf_print_interval = std::chrono::milliseconds(300); // how often should we print stats
		time_start = std::chrono::steady_clock::now();
		time_print_next = time_start + conf_print_interval ; // when should we next time print the stats

		target_speed_Mbit_sec = speed; // [MegaBit/sec]
		target_speed_MByte_microsec = (target_speed_Mbit_sec/8) * (1000*1000); // [Byte/microSec]

		packets_sent=0;
		size_sent= 0;

		auto func_done = [this](decltype(packets_sent) packets_sent) -> bool {
			if (this->count_infinite) return false;
			if (packets_sent >= this->count) return true;
			return false;
		};

		auto limit_time_timepoint = std::chrono::steady_clock::now()
			+ std::chrono::seconds( static_cast<long int>(limit_time_sec));

		while ( ! func_done(packets_sent) ) {
			for (mysize_t b=0; b<burst; ++b) {

				auto buf = boost::asio::buffer(request, request_length); // *

				// [asioflow]
				mysocket.send_to(buf, endpoint); // ***

				++packets_sent;
				size_sent += request_length;
				if ( func_done(packets_sent) ) break;
			} // burst sending done

			auto time_now = std::chrono::steady_clock::now();
			double ellapsed_now =  std::chrono::duration_cast<std::chrono::microseconds>
				( time_now - time_start ).count(); // [microSec, since start]
			auto size_next = size_sent + burst * request_length; // [Byte] that much data will be sent after next burst
			auto ellapsed_target = (static_cast<double>(size_next)*1024*1024) / target_speed_MByte_microsec; // [microSec]
			/*std::cerr << "ellapsed_now=" << ellapsed_now
				<< "  ellapsed_target=" << ellapsed_target
				<< " size_sent=" << size_sent << " size_next=" << size_next
				<< std::endl;
			*/
			auto sleep_target = ellapsed_target - ellapsed_now; // [microSec]
			if (sleep_target > 0) {
				++counter_sleep_yes;
				long int sleep_target_usec = static_cast<long int>(sleep_target);
				// std::cerr << "Will sleep for " << sleep_target_usec << " usec" << std::endl;
				std::this_thread::sleep_for(std::chrono::microseconds( sleep_target_usec ));
			}
			else ++counter_sleep_no;

			if (time_now > time_print_next ) {
				time_print_next = time_now + conf_print_interval ;
				auto speed_totall_Mbit = ( (size_sent*1000.*1000.) / (ellapsed_now*1000*1000) )* 8;

				std::cout << "Sent: " << packets_sent << " pck, " << size_sent << " B" << ". "
					<< speed_totall_Mbit << " Mbit/sec "
					<< " limit=" << target_speed_Mbit_sec << " Mbit/sec = " << target_speed_MByte_microsec << " MB/usec "
					<< " Burst= " << burst << " pck;"
					<< " Sleep: yes=" << counter_sleep_yes << ", no=" << counter_sleep_no
					<< "\n"
				;

				if (limit_time_sec > 0) {
					if (time_now >= limit_time_timepoint) { pfp_info("Time limit reached, exiting");
						break;
					}
				}
				if (limit_data_GiB > 0)  {
					if (size_sent >= (limit_data_GiB*1024*1024*1024)) {
						pfp_info("Size limit reached, exiting");
						break;
					}
				}
			}
		}
		if (!interactive) break;
	}
	return 0;
}

int main(int argc, const char * argv[]) {
	std::cout << std::setprecision(2) << std::setw(6) << std::fixed ;

	c_maintask maintask;

	bool run_remote = false;
	if (argc>=2) {
		if ( std::string(argv[1]) == std::string("remote") ) run_remote = true;
	}

	if (run_remote) {
		pfp_info("Will run remote");
		return maintask.run_remote(argc,argv);
	}
	else {
		pfp_info("Will run (normal, not remote)");
		return maintask.run(argc,argv);
	}
}





