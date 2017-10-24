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

///////////////////////////////////////////////////////////////////////////////////////////
class c_rpc final {
	public:
		c_rpc(std::function<void(const std::string &)> f);
		void start_listen(boost::asio::ip::address_v4 listen_address, unsigned short port);
	private:
		boost::asio::io_service m_io_service;
		boost::asio::ip::tcp::socket m_socket;
		std::function<void(const std::string &)>m_rpc_fun;
		std::string m_input_buffer;
};

c_rpc::c_rpc(std::function<void(const std::string &)> f)
:
	m_io_service(),
	m_socket(m_io_service),
	m_rpc_fun(f),
	m_input_buffer()
{
}

void c_rpc::start_listen(boost::asio::ip::address_v4 listen_address, unsigned short port) {
	boost::asio::ip::tcp::acceptor acceptor(m_io_service, boost::asio::ip::tcp::endpoint(listen_address, port));
	acceptor.accept(m_socket);
	boost::asio::streambuf input_stream;
	const std::string ok_message = "OK";
	while (true) {
		boost::asio::read_until(m_socket, input_stream, '\n');
		std::istream istream(&input_stream);
		std::getline(istream, m_input_buffer);
		m_rpc_fun(m_input_buffer);
		boost::asio::write(m_socket, boost::asio::buffer(ok_message));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////

using boost::asio::ip::udp;
using std::endl;

enum { max_length = 655035 };

using mysize_t = uint64_t; // size_t is too small to count some things, and overflows, e.g. on r-pi

class c_maintask {
	public:
		c_maintask();

		int run(int argc, char *argv[]);
		int run_remote(int argc, char *argv[]);

		void run_rpc_command_string(const std::string & rpc_cmd);

		void print_usage() const;
		void print_help_sendcommand() const;

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
		mysize_t packets_sent, size_sent;

		std::chrono::microseconds conf_print_interval; // how often should we print stats
		std::chrono::steady_clock::time_point time_start;
		std::chrono::steady_clock::time_point time_print_next;

		double target_speed_Mbit_sec;
		double target_speed;

		// conf:
		char *host;
		char *port;
		mysize_t speed;
		std::string message;
		bool interactive;
		mysize_t bytes, count, request_length;
		bool count_infinite; // infinite count sends forever

	//	udp::socket m_remote_sock; ///< local socket to receive remote commands (in remote mode)
};

c_maintask::c_maintask()
: mysocket(io_service), resolver(io_service)
{
}

void c_maintask::print_help_sendcommand() const {
	std::cout << "SendCommand: " << endl
		<< "  foo 0 1    - this will send text foo (1 time)" << std::endl
		<< "  abc 50 1000   - this will send message of letter 'a' repeated 50 times. This msg will be sent 1000 times."
		<<std::endl;
}

void c_maintask::print_usage() const {
		std::cout << std::endl;
		std::cout << "Usage: ./client <host> <port> <max_speed> SendCommand" << std::endl;
		std::cout << "e.g.:  ./client <host> <port> <max_speed> <msg> <msgbytes> <count> " << std::endl;
		std::cout << "e.g.:  ./client 127.0.0.1 9000  999000    foo   1500       -1 " << std::endl;
		std::cout << "e.g.:  ./client remote_cmd 192.168.70.17 9000 " << std::endl;
		std::cout << "or instead allow remote access (remote controll of this sending, WARNING can make your computer spam/DDoS other computer) " << std::endl;
		std::cout << "e.g.:  ./client remote <listen_on_ip><port>  <authorized_ip> <max_time_hours> <pass>  " << std::endl;
		std::cout << "e.g.:  ./client remote 192.168.70.17 19000   192.168.70.16   72 secret1" << std::endl;
		std::cout << "e.g.:  ./client remote 192.168.70.17 19000   0.0.0.0         4  secret1" << std::endl;
		std::cout << "e.g.:  ./client remote       0.0.0.0 19000   0.0.0.0         4  secret1" << std::endl;
		std::cout << "then send TCP text: 'secret1 SEND [args for normal run] \\n'  (\\n means to end the TCP text with a newline)" << std::endl;
		std::cout << "then send TCP text: 'secret1 SEND 127.0.0.1 9000  999000    foo   1500       -1 \\n'" << std::endl;
		std::cout << std::endl;
}


void c_maintask::run_rpc_command_string(const std::string & rpc_cmd) {
}

int c_maintask::run_remote(int argc, char *argv[]) {
	std::cout << "Starting remote server " << std::endl;
	// listen TCP

	// handler:
	// string rpc_text;
	//
	// read ... s  ;  rpc_text += s;



  // NOT NOW
	// NO find first \n... ;  rpc_command = rpc_textsubstr(0, pos_newline);   rcp_text = rpc_text.substr(pos_newline);
	// NO s = ".............. \n ............. \n ............ \n"

	// s = "secret1 SEND 127.0.0.1 9000  999000    foo   1500       -1"

	if (argc < 6) {
		print_usage();
		return 1;
	}

	c_rpc rpc([this](const std::string &rpc_data) {
			std::cout << "rpc data " << rpc_data << '\n';
			run_rpc_command_string(rpc_data);
		}
	);
	boost::asio::ip::address_v4 listen_address = boost::asio::ip::address_v4::from_string(argv[2]);
	unsigned short port = std::atoi(argv[3]);
	rpc.start_listen(listen_address, port); // blocks

	return 0;
}

int c_maintask::run(int argc, char *argv[])
{
	if (argc < 3) {
		print_usage();
		print_help_sendcommand();
		return 1;
	}
	host = argv[1];
	port = argv[2];
	speed = std::stoi(argv[3]);
	burst = 50;

	interactive=true;
	count_infinite=false; // infinute count sends forever

	if (argc >= 2+3+1) {
		interactive=false;
		message = argv[4];
		bytes = atoi(argv[5]);
		double count_exact = atof(argv[6]);
		if (count_exact < 0) {
			count=0; count_infinite=true;
		}
		else {
			count = static_cast<decltype(count)>(count_exact);
			count_infinite=false;
		}
	}

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
		std::cerr << " time(s) an UDP packet: ";
		std::cerr << "datagram: size " << request_length << " B " << "\n";
		// std::cerr << "request=\"" << request << "\"" << "\n";
		std::cerr << "message=\"" << message << "\"" << "\n";
		std::cerr << "data begins with \""
			<< std::string(request, request + std::min(static_cast<size_t>(10),request_length))
			<< "\"" << std::endl;
		std::cerr << endl;

		conf_print_interval = std::chrono::milliseconds(1000); // how often should we print stats
		time_start = std::chrono::steady_clock::now();
		time_print_next = time_start + conf_print_interval ; // when should we next time print the stats

		target_speed_Mbit_sec = speed; // [MegaBit/sec]
		target_speed = (target_speed_Mbit_sec/8); // [Byte/microSec]

		packets_sent=0;
		size_sent= 0;

		auto func_done = [this](decltype(packets_sent) packets_sent) -> bool {
			if (this->count_infinite) return false;
			return (packets_sent >= this->count) ;
		};

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
			auto ellapsed_next = size_next / target_speed; // [microSec]
			auto sleep_next = ellapsed_next - ellapsed_now; // [microSec]
			if (sleep_next > 0) {
				++counter_sleep_yes;
				std::this_thread::sleep_for(std::chrono::microseconds( static_cast<long int>(sleep_next) ));
			}
			else ++counter_sleep_no;

			if (time_now > time_print_next ) {
				time_print_next = time_now + conf_print_interval ;
				auto speed_totall_Mbit = ( (size_sent*1000.*1000.) / (ellapsed_now*1000*1000) )* 8;

				std::cout << "Sent: " << packets_sent << " pck, " << size_sent << " B" << ". "
					<< speed_totall_Mbit << " Mbit/sec "
					<< " limit=" << target_speed_Mbit_sec << " Burst= " << burst << " pck;"
					<< " Sleep: yes=" << counter_sleep_yes << ", no=" << counter_sleep_no
					<< "\n"
				;
			}
		}
		if (!interactive) break;
	}
	return 0;
}


int main(int argc, char *argv[]) {
	std::cout << std::setprecision(2) << std::setw(6) << std::fixed ;

	c_maintask maintask;

	bool run_remote = false;
	if (argc>=2) {
		if ( std::string(argv[1]) == std::string("remote") ) run_remote = true;
	}

	if (run_remote) {
		return maintask.run_remote(argc,argv);
	}
	else {
		return maintask.run(argc,argv);
	}
}





