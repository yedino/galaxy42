/*
This program - as all codes in this project - is totally experimental, likelly has many bugs and exploits!
Do not use it.

Possible ASIO bug (or we did something wrong): see https://svn.boost.org/trac10/ticket/13193

-----------

*/

#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <system_error>

#include <boost/asio.hpp>
#include <iostream>
#include <chrono>
#include <atomic>
#include <mutex>

#include "../src/libs0.hpp" // libs for Antinet

#ifndef ANTINET_PART_OF_YEDINO

#define print_debug(X) { ::std::ostringstream _dbg_oss; _dbg_oss<<__LINE__<<": "<<X<<::std::endl;  ::std::cerr<<_dbg_oss.str(); }

#if 1
#define _dbg4(X) {}
#define _dbg1(X) { if (g_debug) { print_debug(X); } }
#define _note(X) { _dbg1(X); }
#else
#define _dbg4(X) {if(0) { print_debug(X); } }
#define _dbg1(X) {if(0)_dbg4(X);}
#define _note(X) {if(0)_dbg4(X);}
#define _mark(X) {if(0)_dbg4(X);}
#endif

#define _erro(X) { print_debug("\n\n@@@@@@ ERRROR: " << X << "\n\n" ); }
#define _mark(X) { print_debug(    "###### " << X ); }
#define _goal(X) { print_debug(    "------ " << X ); }

#define UsePtr(X) (* (X) )

#endif

#define ANTINET_DEBUG_MODE 1 ///< if 1, then e.g. debug here is enabled, and some asserts/checks are executed

#if ANTINET_IF_DEBUG
#define ANTINET_IF_DEBUG(X) (X);
#else
#define ANTINET_IF_DEBUG(X) ;
#endif



#define addrvoid(X) ( static_cast<const void*>( & (X) ) )


namespace n_netmodel {

bool g_debug = false;

using namespace boost;
using std::vector;
using std::unique_ptr;
using std::make_unique;
using std::string;
using std::endl;

/// Class to access object in a way that exposes race conditions
template <typename T>
class ThreadObject {
	public:
		template<typename... Args> ThreadObject(Args&&... args) : m_obj(std::forward<Args>(args)...) {
			ANTINET_IF_DEBUG( m_test_data=0; );
		}

		T & get() noexcept {
			ANTINET_IF_DEBUG( { m_test_data = (m_test_data+1) % 255; } )
			return m_obj;
		}
		const T & get() const noexcept {
			ANTINET_IF_DEBUG( { m_test_data = (m_test_data+1) % 255; } )
			return m_obj;
		}

		int get_test_data() noexcept { return m_test_data; }

	private:
		T m_obj;
		volatile int m_test_data; ///< write to this to cause (expose) a race condition
};

/// Class that has some object (e.g. a socket) and a strand that protects that object
template <typename T>
class with_strand {
	public:
		template<typename... Args> with_strand(boost::asio::io_service & ios, Args&&... args)
			: m_obj(std::forward<Args>(args)...) , m_strand(ios) {}

		/**
			* Acccess the object #m_obj
			* @warning the caller guarantees that he is calling from a strand (this functin does NOT assert that),
			* it is UB in other case
			*/
		T & get_unsafe_assume_in_strand() { return m_obj; }
		const T & get_unsafe_assume_in_strand() const { return m_obj; }

		asio::io_service::strand get_strand() { return m_strand; }

		template <typename Lambda> auto wrap(Lambda && lambda) {
			return m_strand.wrap( std::move(lambda) );
		}

	private:
		T m_obj;
		asio::io_service::strand m_strand;
};

int g_stage_sleep_time = 0; ///< sleep between stages of startup, was used to debug some race conditions

/// simple timer
struct t_mytime {
	using t_timevalue = std::chrono::time_point<std::chrono::steady_clock>;
	t_timevalue m_time;
	t_mytime() noexcept { }
	t_mytime(std::chrono::time_point<std::chrono::steady_clock> time_) noexcept { m_time = time_; }
};

class c_timerfoo {
	public:
		using t_my_count = long int;
		using t_my_size = long int;

		c_timerfoo();
		void add(t_my_count count, t_my_size size_totall) noexcept; ///< e.g. (3,1024) means we got 3 packets, that in sum have size 1024 B
		std::string get_info() const ;
		void print_info(std::ostream & ostr) const ;

	private:
		std::atomic<t_mytime> m_time_started;
		std::atomic<t_my_count> m_count;
		std::atomic<t_my_size> m_size;
};

c_timerfoo::c_timerfoo() : m_time_started(t_mytime{}), m_count(0), m_size(0) {
}

void c_timerfoo::add(t_my_count count, t_my_size size_totall) noexcept {
	// [counter]
	this->m_count += count;
	this->m_size += size_totall;
	t_mytime time_now( std::chrono::steady_clock::now() );
	t_mytime time_zero;
	this->m_time_started.compare_exchange_strong(
		time_zero,
		time_now
	);
}

std::string c_timerfoo::get_info() const {
	std::ostringstream oss;
	print_info(oss);
	return oss.str();
}

void c_timerfoo::print_info(std::ostream & ostr) const {
	auto time_now = std::chrono::steady_clock::now();
	auto time_started = this->m_time_started.load().m_time;
	t_my_size current_size = this->m_size.load();
	t_my_count current_count = this->m_count.load();

	double ellapsed_sec = ( std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_started) ).count() / 1000.;
	double current_size_speed  = current_size  / ellapsed_sec; // in B/s
	double current_count_speed = current_count / ellapsed_sec; // in B/s

	const double MB = 1*1000*1000;

	int detail=0;
	if (detail>=2) { ostr << std::setw(9) << current_size  << " B "; }
	ostr << std::setw(4) << (current_size_speed/MB)  << " MB/s" ;
	ostr << " ";
	if (detail>=1) { ostr << std::setw(6) << current_count << " p "; }
	ostr << std::setw(4) << (current_count_speed/MB) << " Mp/s" ;
}

std::ostream & operator<<(std::ostream & ostr, c_timerfoo & timer) {
	timer.print_info(ostr);
	return ostr;
}

// ============================================================================

c_timerfoo g_speed_wire_recv;

std::atomic<bool> g_atomic_exit;
std::atomic<int> g_running_tuntap_jobs;

std::atomic<long int> g_state_tuntap2wire_started;
std::atomic<long int> g_state_tuntap_fullbuf;
c_timerfoo g_state_tuntap2wire_in_handler1;
c_timerfoo g_state_tuntap2wire_in_handler2;

// ============================================================================
// Wire
// ============================================================================

/// input buffer, e.g. for reading from wire
struct t_inbuf {
	char m_data[1024];
	static size_t size();
	asio::ip::udp::endpoint m_ep;
};
size_t t_inbuf::size() { return std::extent< decltype(m_data) >::value; }

class c_inbuf_tab {
	public:
		vector<unique_ptr<t_inbuf>> m_inbufs;

		c_inbuf_tab(size_t howmany);
		size_t buffers_count();
		char* addr(size_t ix); ///< thread-safe
		t_inbuf & get(size_t ix); ///< thread-safe
};
c_inbuf_tab::c_inbuf_tab(size_t howmany) {
	for (size_t i=0; i<howmany; ++i) {
		auto newbuff = make_unique<t_inbuf>();
		_note("newbuff at " << addrvoid( *newbuff) );
	//	_dbg1("newbuff before move: " << newbuff.get() );
		m_inbufs.push_back( std::move(newbuff) );
	//	_dbg1("newbuff after move: " << newbuff.get() );
	//_dbg1("tab after move: " << m_inbufs.at(i).get() );
	}
}
size_t c_inbuf_tab::buffers_count() { return m_inbufs.size(); }

char* c_inbuf_tab::addr(size_t ix) {
	return m_inbufs.at(ix)->m_data;
}

t_inbuf & c_inbuf_tab::get(size_t ix) {
	return * m_inbufs.at(ix) ; // thread-safe: access to vector (read only)
}

void handler_signal_term(const boost::system::error_code& error , int signal_number)
{
	_goal("Signal! (control-C?) " << signal_number);
	UNUSED(error);
	g_atomic_exit = true;
}
// ==================================================================
// crypto
// ==================================================================
#include <sodium.h>
namespace crypto {
	std::array<unsigned char, crypto_stream_NONCEBYTES> nonce;
	std::array<unsigned char, crypto_stream_KEYBYTES> secret_key;
	void init() {
		if (sodium_init() == -1) {
			throw std::runtime_error("sodium init error");
		}
		randombytes_buf(nonce.data(), nonce.size());
		randombytes_buf(secret_key.data(), secret_key.size());
	}
}

// ==================================================================
// wire receiving
// ==================================================================

enum class e_algo_receive {
	after_first_read, after_next_read, // handler when we got the date -, need to read it and use it, decrypt it
	after_processing_done, // handler when we are now ordered to restart the read
};

int cfg_test_crypto_task=0; // global option

enum class t_mt_method {
	mt_unset=0, // not set
	mt_strand=1, // strands, wrap
	mt_mutex=2, // mutex, lock
};
t_mt_method cfg_mt_method = t_mt_method::mt_unset;

void handler_receive(const e_algo_receive algo_step, const boost::system::error_code ec, std::size_t bytes_transferred,
	with_strand< ThreadObject<asio::ip::udp::socket> > & mysocket,
	c_inbuf_tab & inbuf_tab, size_t inbuf_nr,
	std::mutex & mutex_handlerflow_socket)
{
	if (!! ec) {
		_note("Handler hit error, ec="<<ec.message());
		return;
	}

	auto & inbuf = inbuf_tab.get(inbuf_nr);
	_note("handler for inbuf_nr="<<inbuf_nr<<" for tab at " << static_cast<void*>(&inbuf_tab)
		<< " inbuf at " << static_cast<void*>( & inbuf)
		<< " from remote IP " << inbuf.m_ep << " bytes_transferred="<<bytes_transferred
		<< " read: ["<<std::string( & inbuf.m_data[0] , bytes_transferred)<<"]"
	);

	if ((algo_step==e_algo_receive::after_first_read) || (algo_step==e_algo_receive::after_next_read)) {
		g_speed_wire_recv.add(1, bytes_transferred); // [counter] inc

		static const char * marker = "exit";
		static const size_t marker_len = strlen(marker);

		if (std::strncmp( &inbuf.m_data[0] , marker , std::min(bytes_transferred,marker_len) )==0) {
			if (bytes_transferred == marker_len) {
				_note("Message is EXIT, will exit");
				g_atomic_exit=true;
			}
		}

		// fake "decrypt/encrypt" operation
		if (cfg_test_crypto_task > 0) {
			unsigned char bbb=0;
			for (int j=0; j<cfg_test_crypto_task; ++j) {
				unsigned char aaa = j%5;
				for (size_t pos=0; pos<bytes_transferred; ++pos) {
					aaa ^= inbuf.m_data[pos] & inbuf.m_data[ (pos*(j+2))%bytes_transferred ] & j;
				}
				bbb ^= aaa;
			}
			auto volatile rrr = bbb;
			if (rrr==0) _note("rrr="<<static_cast<int>(rrr));
		}
		else if (cfg_test_crypto_task < 0) {
			int ret = crypto_stream_xor(
				reinterpret_cast<unsigned char *>(&(inbuf.m_data[0])), // out
				reinterpret_cast<unsigned char *>(&(inbuf.m_data[0])), // in
				std::extent<decltype(inbuf.m_data)>::value, // in len
				crypto::nonce.data(), // nonce
				crypto::secret_key.data() // secret key
			);
			if (!ret) _erro("Crypto error: " << ret);
		}
		else {
			// nothing. Just avoid warnings / deadcode optimize / unused
			unsigned char volatile xxx;
			xxx=inbuf.m_data[0];
			unsigned char volatile yyy = xxx;
			if (yyy == 0) ++yyy;
		}

		// [asioflow]
		if (cfg_mt_method == t_mt_method::mt_strand) {
			mysocket.get_strand().post(
				//mysocket.wrap(
				[&mysocket, &inbuf_tab , inbuf_nr, & mutex_handlerflow_socket]()
				{
					_dbg1("Handler (restart read)");
					handler_receive(e_algo_receive::after_processing_done, boost::system::error_code(),0, mysocket, inbuf_tab,inbuf_nr, mutex_handlerflow_socket);
				}
				//)
			);
		}
		else if (cfg_mt_method == t_mt_method::mt_mutex) {
			mysocket.get_unsafe_assume_in_strand().get().get_io_service().post(
				[&mysocket, &inbuf_tab , inbuf_nr, & mutex_handlerflow_socket]()
				{
					_dbg1("Handler (restart read)");
					handler_receive(e_algo_receive::after_processing_done, boost::system::error_code(),0, mysocket, inbuf_tab,inbuf_nr, mutex_handlerflow_socket);
				}
			);
		}
		else throw std::runtime_error("unsupported mt");
	} // first_read or next_read

	// ---

	else if (algo_step==e_algo_receive::after_processing_done) {
		_dbg1("Restarting async read, on mysocket="<<addrvoid(mysocket));
		char* inbuf_data = & inbuf.m_data[0] ;
		auto inbuf_asio = asio::buffer( inbuf_data  , std::extent<decltype(inbuf.m_data)>::value );
		assert( asio::buffer_size( inbuf_asio ) > 0 );
		_dbg4("buffer size is: " << asio::buffer_size( inbuf_asio ) );

		if (cfg_mt_method == t_mt_method::mt_mutex) {
			std::lock_guard< std::mutex > lg( mutex_handlerflow_socket ); // *** LOCK ***
			// [asioflow]
			mysocket.get_unsafe_assume_in_strand() // in this block, we are using LOCKING so this is safe.
				.get()
				.async_receive_from( inbuf_asio , inbuf_tab.get(inbuf_nr).m_ep ,
					[&mysocket, &inbuf_tab , inbuf_nr, & mutex_handlerflow_socket](const boost::system::error_code & ec, std::size_t bytes_transferred_again)
					{
						_dbg1("Handler (again), size="<<bytes_transferred_again<<", ec="<<ec.message());
						handler_receive(e_algo_receive::after_next_read, ec,bytes_transferred_again, mysocket, inbuf_tab,inbuf_nr, mutex_handlerflow_socket);
					}
			);
		}
		else if (cfg_mt_method == t_mt_method::mt_strand) {
			// [asioflow]
			mysocket.get_unsafe_assume_in_strand() // we are called in a handler that should be wrapped, so this is safe
				.get()
				.async_receive_from( inbuf_asio , inbuf_tab.get(inbuf_nr).m_ep ,
					[&mysocket, &inbuf_tab , inbuf_nr, & mutex_handlerflow_socket](const boost::system::error_code & ec, std::size_t bytes_transferred_again)
					{
						_dbg1("Handler (again), size="<<bytes_transferred_again<<", ec="<<ec.message());
						handler_receive(e_algo_receive::after_next_read, ec,bytes_transferred_again, mysocket, inbuf_tab,inbuf_nr, mutex_handlerflow_socket);
					}
			);
		}
		else throw std::runtime_error("unsupported mt");

		_dbg1("Restarting async read - done");
	} // restart_read
	else throw std::runtime_error("Unknown state of algo.");
}

int safe_atoi(const std::string & s) {
	return atoi(s.c_str());
}

string yesno(bool yes) {
	if (yes) return "ENABLED";
	return "no";
}

struct t_mycmdline {
	vector<string> m_arg;
	vector<bool> m_used;
	void add(const string & arg) { m_arg.push_back(arg); m_used.push_back(false); }
};

// T must be integral. Will set target_var with the number from "foo=42", when name="foo". Provide the cmdline;
// Throws if required==true but value is not found.
// Returns if the value was found.
template<typename T>
bool set_from_cmdline(T & target_var, const string & name, t_mycmdline &cmdline, bool required=false) {
	for (size_t ix=0; ix<cmdline.m_arg.size(); ++ix) {
		const string & arg = cmdline.m_arg.at(ix);
		auto pos=arg.find('=');
		if (pos==string::npos) continue;
		if (pos>=arg.size()) continue;
		string one_name=arg.substr(0,pos);
		if (one_name == name) {
			string val=arg.substr(pos+1);
			_note("argument ["<<name<<"] = ["<<val<<"] (ix="<<ix<<")");
			target_var = safe_atoi(val);
			cmdline.m_used.at(ix) = true; // it was used
			return true;
		}
	}
	if (required) { _erro("You need to provide argument '"<<name<<"'."); throw std::runtime_error("Missing arg"); }
	return false;
}

int get_from_cmdline(const string & name, t_mycmdline &cmdline) {
	int the_val=0;
	set_from_cmdline(the_val, name, cmdline, true);
	return the_val;
}

int get_from_cmdline(const string & name, t_mycmdline &cmdline, int def) {
	int the_val=0;
	bool got_it = set_from_cmdline(the_val, name, cmdline, false);
	if (!got_it) the_val=def;
	return the_val;
}

void asiotest_udpserv(std::vector<std::string> options) {
	// the main "loop"

	g_atomic_exit=false;
	g_running_tuntap_jobs=0;

	g_state_tuntap2wire_started=0;
	g_state_tuntap_fullbuf=0;

	auto func_show_usage = []() {
		std::cout << "\nUsage: \n"
		<< "./a.out 192.168.1.107 2345 crypto=0 wire_buf=4 wire_sock=2 wire_ios=2 wire_ios_thr=2 tuntap_weld=2 tuntap_sock=1 tuntap_ios=1 tuntap_ios_thr=2 tuntap_weld_sleep=1 tuntap_block mt_strand \n"
		<< "Also can ADD this options:\n"
		<< "  mt_strand/mt_mutex\n"
		<< "  tuntap_block/tuntap_async\n"
		<< "  mport debug\n"
		<< "Crypto: -1 is some crypto test; n (n>0) is reading all bytes; n==0 is nothing \n"
		<< "See code for more details, search func_cmdline.\n"
		<< std::endl;
	};

	t_mycmdline mycmdline;
	for (const string & arg : options) mycmdline.add(arg);
	auto func_cmdline = [&mycmdline](const string &name) -> int { return get_from_cmdline(name,mycmdline); } ;
	auto func_cmdline_def = [&mycmdline](const string &name, int def) -> int { return get_from_cmdline(name,mycmdline,def); } ;

	const int cfg_num_inbuf = func_cmdline("wire_buf"); // e.g. 32 ; this is also the number of flows (wire/p2p connections)
	const int cfg_num_socket_wire = func_cmdline("wire_sock"); // 2 ; number of sockets - wire (p2p)
	const int cfg_buf_socket_spread = func_cmdline_def("wire_spread",0); // 0 is: (buf0,sock0),(b1,s1),(b2,s0),(b3,s1),(b4s0) ; 1 is (b0,s0),(b1,s0),(b2,s1),(b3,s1)

	const int cfg_port_faketuntap = 2345;

	const int cfg_num_ios = func_cmdline("wire_ios"); // 4
	const int cfg_num_thread_per_ios = func_cmdline("wire_ios_thr"); // 16

	cfg_test_crypto_task = func_cmdline("crypto"); // 10

	const int cfg_num_weld_tuntap = func_cmdline("tuntap_weld"); // safe_atoi(options.at(6)); // 16?
	const int cfg_num_socket_tuntap = func_cmdline("tuntap_sock"); // 4?

	int cfg_tuntap_buf_sleep = -1; // sleep in tuntap loop waiting for buffer, see code for meaning

	const int cfg_tuntap_ios = func_cmdline("tuntap_ios"); // use ios for tuntap sockets: -1=use first one from general/wire ios;
	const int cfg_tuntap_ios_threads_per_one = func_cmdline("tuntap_ios_thr"); // for each ios of tuntap (if any ios are created for tuntap) how many threads to .run it in

	cfg_tuntap_buf_sleep = func_cmdline("tuntap_weld_sleep");

	vector<asio::ip::udp::endpoint> peer_pegs;
	//peer_pegs.emplace_back( asio::ip::address_v4::from_string("127.0.0.1") , 9000 );
	_note("Adding peer");
	{
		auto opt_addr = options.at(0);
		auto opt_port = options.at(1);
		_note("Peer address ["<<opt_addr<<"] and prot ["<<opt_port<<"]");
		peer_pegs.emplace_back(
			asio::ip::address_v4::from_string(opt_addr) ,
			safe_atoi(opt_port));
	}
	_note("Got peer(s) " << peer_pegs.size());

	bool tuntap_set=false;
	bool cfg_tuntap_blocking=false;
	for (const string & arg : options) if (arg=="tuntap_block")  {  tuntap_set=true;  cfg_tuntap_blocking=true;  }
	for (const string & arg : options) if (arg=="tuntap_async")  {  tuntap_set=true; cfg_tuntap_blocking=false; }
	if (!tuntap_set) {
		func_show_usage();
		std::cerr << endl << "ERROR: You must add option either tuntap_block or tuntap_async (or other option of this family, see source)" << endl;
		throw std::runtime_error("Must set tuntap method");
	}

	for (const string & arg : options) if (arg=="mt_strand")
		{ assert(cfg_mt_method==t_mt_method::mt_unset); cfg_mt_method=t_mt_method::mt_strand; }
	for (const string & arg : options) if (arg=="mt_mutex")
		{ assert(cfg_mt_method==t_mt_method::mt_unset); cfg_mt_method=t_mt_method::mt_mutex; }

	if (cfg_mt_method == t_mt_method::mt_unset) {
		func_show_usage();
		std::cerr << endl << "ERROR: You must add option either mt_strand or mt_mutex (or other option of this family, see source)" << endl;
		throw std::runtime_error("Must set mt method");
	}

	bool cfg_port_multiport = false;
	for (const string & arg : options) if (arg=="mport") cfg_port_multiport=true;

	for (size_t ix=0; ix<mycmdline.m_arg.size(); ++ix) {
		const string & arg = mycmdline.m_arg.at(ix);
		auto pos=arg.find('=');
		if ( (pos!=string::npos) && (mycmdline.m_used.at(ix)==false) ) {
			_erro("Unused/unknown argument: " << arg << " (ix="<<ix<<")" );
			throw std::runtime_error("Unused argument");
		}
	}

	_goal("Parsing options done");

	auto func_show_summary = [&]() {
		std::ostringstream oss;
		oss<<"Summary: " << endl
			<< "  inbufs: " << cfg_num_inbuf << endl
			<< "  socket: " << cfg_num_socket_wire << " in spread: " << cfg_buf_socket_spread << endl
			<< "  ios: " << cfg_num_ios << " per each there are threads: " << cfg_num_thread_per_ios << endl
			<< "  option: " << "mport="<<yesno(cfg_port_multiport)<<" " << endl
			<< endl
			<< "Program functions:" << endl
			<< "  reading UDP P2P: yes (async multithread as above)" << endl
			<< "    decrypt E2E: faked (task=" << cfg_test_crypto_task << ")" << endl
			<< "    decrypt P2P: equals E2E" << endl
			<< "    re-route received P2P: no, throw away" << endl
			<< "    re-route received P2P, do P2P-crypto: no" << endl
			<< "    consume received P2P into our endpoint TUN: no, throw away" << endl
			<< "  reading local TUNTAP: *TODO* faked as UDP localhost port=" << cfg_port_faketuntap << endl
			<< "    IOS: "<<cfg_tuntap_ios << " * threads="<<cfg_tuntap_ios_threads_per_one << endl
			<< "    details: sleep="<<cfg_tuntap_buf_sleep << endl
			<< "    using weld buffers: " << cfg_num_weld_tuntap << endl
			<< "    using sockets: " << cfg_num_socket_tuntap << " that are: " << (cfg_tuntap_blocking ? "BLOCKING" : "async") << endl
			<< "    encrypt E2E: no" << endl
			<< "    encrypt P2P: equals E2E" << endl
			<< "    send out endpoint data from TUN: YES, to peers: ";
		for (const auto &peg : peer_pegs) oss << peg << " ";
		oss << endl;
		_goal(oss.str());
	};
	func_show_summary();

	_goal("Starting test. Crypto task: "<<cfg_test_crypto_task);

	_note("Create ios (general)");
	asio::io_service ios_general;
	auto ios_general_work = make_unique< asio::io_service::work >(ios_general);

	_note("Create ios (WIRE)");
	std::vector<std::unique_ptr<asio::io_service>> ios_wire;
	std::vector<std::unique_ptr<asio::io_service::work>> ios_wire_work;
	for (int i=0; i<cfg_num_ios; ++i) {
		_goal("Creating ios (WIRE) nr "<<i);
		ios_wire.emplace_back( std::make_unique<asio::io_service>() );
		ios_wire_work.emplace_back( std::make_unique<asio::io_service::work>( * ios_wire.back() ) );
		_goal("Creating ios (WIRE) nr "<<i<<" - done");
	}

	_note("Create ios (TUNTAP)");
	std::vector<std::unique_ptr<asio::io_service>> ios_tuntap;
	std::vector<std::unique_ptr<asio::io_service::work>> ios_tuntap_work;
	for (int i=0; i<cfg_tuntap_ios; ++i) {
		_goal("Creating ios (TUNTAP) nr "<<i);
		ios_tuntap.emplace_back( std::make_unique<asio::io_service>() );
		ios_tuntap_work.emplace_back( std::make_unique<asio::io_service::work>( * ios_tuntap.back() ) );
		_goal("Creating ios (TUNTAP) nr "<<i<<" - done");
	}

	boost::asio::signal_set signals( ios_general, SIGINT);
	signals.async_wait( handler_signal_term );
	std::thread ios_general_thread([&ios_general](){
		ios_general.run();
	});

	vector<std::thread> ios_wire_thread;
	for (int ios_nr = 0; ios_nr < cfg_num_ios; ++ios_nr) {
		for (int ios_thread=0; ios_thread<cfg_num_thread_per_ios; ++ios_thread) {
			_goal("WIRE: start worker: ios_nr=" << ios_nr << " ios_thread=" << ios_thread);
			std::thread thread_run(
				[&ios_wire, ios_thread, ios_nr] {
					while (!g_atomic_exit) {
						ios_wire.at( ios_nr )->run(); // <=== this blocks, for entire main loop, and runs (async) handlers here
						_note("WIRE: ios worker run (ios_thread="<<ios_thread<<" on ios_nr=" << ios_nr <<") is done... will restat?");
						std::this_thread::sleep_for( std::chrono::milliseconds(100) );
					}
					_note("WIRE:ios (wire) worker run (ios_thread="<<ios_thread<<") - COMPLETE");
				}
			);
			ios_wire_thread.push_back( std::move( thread_run ) );
		}
	}
	_note("WIRE: ios threads are running.");

	vector<std::thread> ios_tuntap_thread;
	for (int ios_nr = 0; ios_nr < cfg_tuntap_ios; ++ios_nr) {
		for (int ios_thread=0; ios_thread<cfg_tuntap_ios_threads_per_one; ++ios_thread) {
			_goal("TUNTAP: start worker: ios_nr=" << ios_nr << " ios_thread=" << ios_thread);
			std::thread thread_run(
				[&ios_tuntap, ios_thread, ios_nr] {
					while (!g_atomic_exit) {
						ios_tuntap.at( ios_nr )->run(); // <=== this blocks, for entire main loop, and runs (async) handlers here
						_note("TUNTAP: ios worker run (ios_thread="<<ios_thread<<" on ios_nr=" << ios_nr <<") is done... will restat?");
						std::this_thread::sleep_for( std::chrono::milliseconds(100) );
					}
					_note("TUNTAP:ios (tuntap) worker run (ios_thread="<<ios_thread<<") - COMPLETE");
				}
			);
			ios_tuntap_thread.push_back( std::move( thread_run ) );
		}
	}
	_note("TUNTAP: ios threads are running.");


	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );
	_goal("All ios run are running");
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );

	// --- tuntap classes / data ---
	const int cfg_size_tuntap_maxread=9000;
	const int cfg_size_tuntap_buf=cfg_size_tuntap_maxread * 2;

	static const size_t fragment_pos_max=32;
	struct c_weld {
		unsigned char m_buf[cfg_size_tuntap_buf];

		uint16_t m_fragment_pos[fragment_pos_max]; ///< positions of ends fragments,
		/// as constant-size array, with separate index in it
		/// if array has values {5,10,11} then fragments are ranges: [0..4], [5..9], [10]

		uint16_t m_fragment_pos_ix; ///< index in above array

		size_t m_pos; ///< if ==0, then nothing yet is written; If ==10 then 0..9 is written, 10..end is free
		bool m_reserved; ///< do some thread now read this now?

		c_weld() { clear(); }
		void clear() {
			m_reserved=false;
			m_pos=0;
			m_fragment_pos_ix=0;
		}

		void add_fragment(uint16_t size) {
			assert(m_fragment_pos_ix < fragment_pos_max); // can not add anything since no place to store indexes

			// e.g. buf=10 (array is 0..9), m_pos=0 (nothing used), size=10 -> allowed
			assert(size <= cfg_size_tuntap_buf - m_pos);

			m_pos += size;
			m_fragment_pos[ m_fragment_pos_ix ] = m_pos;
			++m_fragment_pos_ix;
		}
		size_t space_left() const {
			if (m_fragment_pos_ix>=fragment_pos_max) return 0; // can not add anything since no place to store indexes
			return cfg_size_tuntap_buf - m_pos;
		}
		unsigned char * addr_at_pos() { return & m_buf[m_pos]; }
		unsigned char * addr_all() { return & m_buf[0]; }
	};

	// --- welds var ---
	vector<c_weld> welds;
	std::mutex welds_mutex;


	// stop / show stats
	_goal("The stop thread"); // exit flag --> ios.stop()
	std::thread thread_stop(
		[&ios_general,&ios_wire,&ios_tuntap, &ios_general_work, &ios_wire_work, &ios_tuntap_work, &welds, &welds_mutex] {
			for (int i=0; true; ++i) {
				std::this_thread::sleep_for( std::chrono::milliseconds(500) );

				// [counter] read
				std::ostringstream oss;
				oss << "Loop. ";
				oss << "Wire: RECV={" << g_speed_wire_recv << "}";
				oss << "; ";
				oss << "Tuntap: ";
				oss << "start="<<g_state_tuntap2wire_started<<' ';
				oss << "h1={"<<g_state_tuntap2wire_in_handler1<<"} ";
				oss <<" h2={"<<g_state_tuntap2wire_in_handler2<<"} ";
				oss <<" fullBuf="<<g_state_tuntap_fullbuf<<" ";
				oss << "; ";
				oss << "Welds: ";
				{
					std::lock_guard<std::mutex> lg(welds_mutex);
					for (const auto & weld : welds) {
						oss << "[" << weld.space_left() << " " << (weld.m_reserved ? "RESE" : "idle") << "]";
					}
				}
				_goal(oss.str());
				if (g_atomic_exit) {
					_note("Exit flag is set, exiting loop and will stop program");
					break;
				}
			}

			/*
			// do we need to stop work while we have ios stop?
			_goal("Exit: so will stop work...");
			// .reset resets the pointer, resulting in calling destructor on work, so work ends (and ios can exit)
			// thread safe, no one else accesses this work objects - [TODO] except for the init code, sync with it?
			for (auto & one_work : ios_wire_work) one_work.reset();
			for (auto & one_work : ios_tuntap) one_work.reset();
			{ auto & one_work = ios_general_work; one_work.reset(); }
			*/

			_goal("Exit: so will stop ios...");
			// using ios->stop - is thread safe, asio::io_service is TS for most functions
			for (auto & one_ios : ios_wire) {
				one_ios->stop();
			}
			for (auto & one_ios : ios_tuntap) {
				one_ios->stop(); // thread safe, asio::io_service is TS for most functions
			}
			{ auto & one_ios = ios_general;
				one_ios.stop(); // thread safe, asio::io_service is TS for most functions
			}
		}
	);

	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );
	_goal("Stop threat running");
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );

	// sockets for (fake-)TUN connections:
	vector<with_strand<ThreadObject<asio::ip::udp::socket>>> tuntap_socket;
	for (int nr_sock=0; nr_sock<cfg_num_socket_tuntap; ++nr_sock) {
		int port_nr = cfg_port_faketuntap;
		if (cfg_port_multiport) port_nr += nr_sock;
		_note("Creating TUNTAP socket #"<<nr_sock<<" on port " << port_nr);
		auto func_select_ios = [cfg_tuntap_ios, &ios_wire, &ios_tuntap, nr_sock]() -> asio::io_service & {
			if (cfg_tuntap_ios==-2) {
				_note("TUNTAP - selecting for socket="<<nr_sock<<" an IOS from global/wire ios (select from all of them)");
				return * ios_wire.at( nr_sock % ios_wire.size() );
			}
			if (cfg_tuntap_ios==-1) {
				_note("TUNTAP - selecting for socket="<<nr_sock<<" an IOS from global/wire ios (first one)");
				return * ios_wire.at(0);
			}
			if (cfg_tuntap_ios>0) {
				_note("TUNTAP - selecting for socket="<<nr_sock<<" an IOS from TUNTAP own ios");
				return * ios_tuntap.at( nr_sock % ios_tuntap.size());
			}
			throw std::runtime_error("My error in selecting ios for tuntap");
		};
		auto & one_ios = func_select_ios();
		auto & socket_array = tuntap_socket;
		socket_array.push_back( with_strand<ThreadObject<boost::asio::ip::udp::socket>>(one_ios, one_ios) );
		boost::asio::ip::udp::socket & thesocket = socket_array.back().get_unsafe_assume_in_strand().get();

		thesocket.open( asio::ip::udp::v4() );
		// thesocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		thesocket.bind( asio::ip::udp::endpoint( asio::ip::address_v4::any() , port_nr ) );
	}

	// sockets for wire p2p connections:
	vector<with_strand<ThreadObject<asio::ip::udp::socket>>> wire_socket;
	c_inbuf_tab inbuf_tab(cfg_num_inbuf);

	for (int nr_sock=0; nr_sock<cfg_num_socket_wire; ++nr_sock) {
		int port_nr = 9000;
		if (cfg_port_multiport) port_nr += nr_sock;
		_note("Creating wire (P2P) socket #"<<nr_sock<<" on port " << port_nr);
		//wire_socket.push_back({ios,ios}); // active udp // <--- TODO why not?
		auto & one_ios = ios_wire.at( nr_sock % ios_wire.size() );

		auto & socket_array = wire_socket;
		socket_array.push_back( with_strand<ThreadObject<boost::asio::ip::udp::socket>>(*one_ios, *one_ios) );
		boost::asio::ip::udp::socket & thesocket = socket_array.back().get_unsafe_assume_in_strand().get();

		thesocket.open( asio::ip::udp::v4() );
		// thesocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		thesocket.bind( asio::ip::udp::endpoint( asio::ip::address_v4::any() , port_nr ) );
	}

	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );
	_goal("TUNTAP and WIRE sockets are ready");
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );

	asio::ip::udp::endpoint remote_ep;

	std::mutex mutex_handlerflow_socket_wire;

	// ---> tuntap: blocking version seems faster <---

	{
		std::lock_guard<std::mutex> lg(welds_mutex);
		for (int i=0; i<cfg_num_weld_tuntap; ++i) welds.push_back( c_weld() );
	}

	vector<std::thread> tuntap_flow;


	// tuntap: DO WORK
	for (int tuntap_socket_nr=0; tuntap_socket_nr<cfg_num_socket_tuntap; ++tuntap_socket_nr) {
		_mark("Creating workflow (blocking - thread) for tuntap, socket="<<tuntap_socket_nr);

		std::thread thr = std::thread(
			[tuntap_socket_nr, &tuntap_socket, &welds, &welds_mutex, &wire_socket, &peer_pegs, cfg_tuntap_buf_sleep]()
			{
				++g_running_tuntap_jobs;
				int my_random = (tuntap_socket_nr*437213)%38132 + std::rand();

				auto func_send_weld = [&my_random, tuntap_socket_nr, &wire_socket, &peer_pegs, &welds, &welds_mutex](int send_weld_nr) { // lambda
					// select wire
					int wire_socket_nr = ((my_random*4823)%4913) % wire_socket.size(); // TODO better pseudo-random
					++my_random;
					_dbg4("TUNTAP sending out the data from tuntap socket="<<tuntap_socket_nr
						<<" via wire_socket_nr="<<wire_socket_nr);

					// [thread] this is SAFE probably, as we read-only access the peer_pegs (that is not changing)
					asio::ip::udp::endpoint peer_peg = peer_pegs.at(0);

					auto & mysocket = wire_socket.at(wire_socket_nr);
					mysocket.get_strand().post(
						// mysocket.wrap(
							[wire_socket_nr, &wire_socket, &welds, &welds_mutex, send_weld_nr, peer_peg]() {
								auto & weld = welds.at(send_weld_nr);
								size_t send_size = weld.m_pos;

								_dbg4("TUNTAP-WIRE handler1 (in strand). TUNTAP->WIRE will be now sent."
									<< " weld="<<send_weld_nr<<" wire-socket="<<wire_socket_nr);
								g_state_tuntap2wire_in_handler1.add(1, send_size); // [counter]

								auto & wire = wire_socket.at(wire_socket_nr);
								auto send_buf_asio = asio::buffer( weld.addr_all() , send_size );

								wire.get_unsafe_assume_in_strand().get().async_send_to(
									send_buf_asio,
									peer_peg,
									[send_weld_nr, wire_socket_nr, &welds, &welds_mutex](const boost::system::error_code & ec, std::size_t bytes_transferred)
									{
										_dbg4("TUNTAP-WIRE handler2 (sent done). ec="<<ec.message()<<"."
											<< " weld="<<send_weld_nr<<" wire-socket="<<wire_socket_nr
										);
										g_state_tuntap2wire_in_handler2.add(1, bytes_transferred);
										std::lock_guard<std::mutex> lg(welds_mutex); // lock
										auto & weld = welds.at(send_weld_nr);
										weld.clear();
									}
								); // asio send
								_dbg4("TUNTAP-WIRE handler1 (in strand) - ok STARTED the handler2. Socket "<<wire_socket_nr<<" weld " <<wire_socket_nr << " - ASYNC STARTED");
							} // delayed TUNTAP->WIRE
						// ) // wrap
					); // start(post) handler: TUNTAP->WIRE start

					_dbg4("TUNTAP-WIRE posted: weld=" << wire_socket_nr << " to P2P socket="<<wire_socket_nr);
					++g_state_tuntap2wire_started;

				}; // send the full weld

				while (!g_atomic_exit) {
					auto & one_socket = tuntap_socket.at(tuntap_socket_nr);
					_note("TUNTAP reading");

					size_t found_ix=0;
					bool found_any=false;

					{ // lock to find and reserve buffer a weld
						std::lock_guard<std::mutex> lg(welds_mutex);

						for (size_t i=0; i<welds.size(); ++i) {
							if (! welds.at(i).m_reserved) {
								if (welds.at(i).space_left() >= cfg_size_tuntap_maxread) {
									found_ix=i; found_any=true;
									break;
								}
							}
						}

						if (found_any) {
							auto & weld = welds.at(found_ix);
							weld.m_reserved=true; // we are using it now
						}
						else {
							_dbg4("No free tuntap buffers! - fullbuffer!");
							++g_state_tuntap_fullbuf;
							func_send_weld(0); // TODO choose weld
							// forced send
							continue ; // <---
						}
					} // lock operations on welds

					auto & found_weld = welds.at(found_ix);
					size_t receive_size = found_weld.space_left();
					assert(receive_size>0);
					void * buf_ptr = reinterpret_cast<void*>(found_weld.addr_at_pos());
					assert(buf_ptr);
					auto buf_asio = asio::buffer( buf_ptr , receive_size );

					try {
						_dbg4("TUNTAP read, on tuntap_socket_nr="<<tuntap_socket_nr<<" socket="<<addrvoid(one_socket)<<" "
							<<"into weld "<<found_ix<<" "
							<< "buffer size is: " << asio::buffer_size( buf_asio ) << " buf_ptr="<<buf_ptr);
						asio::ip::udp::endpoint ep;

						// [asioflow] read *** blocking
						auto read_size = size_t { one_socket.get_unsafe_assume_in_strand().get().receive_from(buf_asio, ep) };

						_dbg4("TUNTAP ***BLOCKING READ DONE*** socket="<<tuntap_socket_nr<<": got data from ep="<<ep<<" read_size="<<read_size
						<<" weld "<<found_ix<<"\n\n");

						// process data, and un-reserve it so that others can add more to it
						{ // lock
							std::lock_guard<std::mutex> lg(welds_mutex);
							c_weld & the_weld = welds.at(found_ix); // optimize: no need for mutex for this one
							the_weld.add_fragment(read_size);

							bool should_send = ! (the_weld.space_left() >= cfg_size_tuntap_maxread) ;
							_dbg1("TUNTAP (weld "<<found_ix<<") decided to: " << (should_send ? "SEND-NOW" : "not-send-yet")
								<< " space left " << the_weld.space_left() << " vs needed space " << cfg_size_tuntap_maxread);

							if (should_send) { // almost full -> so we send
								func_send_weld(found_ix);
							}
							else { // do not send. weld extended with data
								_dbg4("Removing reservation on weld " << found_ix);
								the_weld.m_reserved=false;
							}
						} // lock to un-reserve

					}
					catch (std::exception &ex) {
						_erro("Error in TUNTAP lambda: "<<ex.what());
						std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
					}
					catch (...) {
						_erro("Error in TUNTAP lambda - unknown");
						std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
					}

					// process the TUN read data TODO
				} // loop forever

				--g_running_tuntap_jobs;
			} // the lambda
		);

		tuntap_flow.push_back( std::move(thr) );

	};
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );
	_goal("TUNTAP workflows are running");
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );

	// wire P2P: add first work - handler-flow
	auto func_spawn_flow_wire = [&](int inbuf_nr, int socket_nr_raw) {
		assert(inbuf_nr >= 0);
		assert(socket_nr_raw >= 0);
		int socket_nr = socket_nr_raw % wire_socket.size(); // spread it (rotate)
		_mark("Creating workflow: buf="<<inbuf_nr<<" socket="<<socket_nr);

		auto inbuf_asio = asio::buffer( inbuf_tab.addr(inbuf_nr) , t_inbuf::size() );
		_dbg1("buffer size is: " << asio::buffer_size( inbuf_asio ) );
		_dbg1("async read, on mysocket="<<addrvoid(wire_socket));
		{
			// std::lock_guard< std::mutex > lg( mutex_handlerflow_socket ); // LOCK

			auto & this_socket_and_strand = wire_socket.at(socket_nr);

			// [asioflow]
			this_socket_and_strand.get_strand().post([&, inbuf_nr] {
				this_socket_and_strand.get_unsafe_assume_in_strand().get().async_receive_from( inbuf_asio , inbuf_tab.get(inbuf_nr).m_ep ,
						[&this_socket_and_strand, &inbuf_tab , inbuf_nr, &mutex_handlerflow_socket_wire](const boost::system::error_code & ec, std::size_t bytes_transferred) {
							_dbg1("Handler (FIRST), size="<<bytes_transferred);
							handler_receive(e_algo_receive::after_first_read, ec,bytes_transferred, this_socket_and_strand, inbuf_tab,inbuf_nr, mutex_handlerflow_socket_wire);
						}
					); // start async
				} // post lambda
			); // post
		}
	} ;

	if (cfg_buf_socket_spread==0) {
		for (int inbuf_nr = 0; inbuf_nr<cfg_num_inbuf; ++inbuf_nr) {	func_spawn_flow_wire( inbuf_nr , inbuf_nr); }
	}
	else if (cfg_buf_socket_spread==1) {
		for (int inbuf_nr = 0; inbuf_nr<cfg_num_inbuf; ++inbuf_nr) {
			int socket_nr = static_cast<int>( inbuf_nr / static_cast<float>(cfg_num_inbuf ) * cfg_num_socket_wire );
			// int socket_nr = static_cast<int>( inbuf_nr / static_cast<float>(inbuf_tab.buffers_count() ) * wire_socket.size() );
			func_spawn_flow_wire( inbuf_nr , socket_nr );
		}
	}

	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );
	_goal("All started");
	std::this_thread::sleep_for( std::chrono::milliseconds(g_stage_sleep_time) );

	func_show_summary();

	_goal("Waiting for all threads to end");
	_goal("Join stop thread");
	thread_stop.join();

	_goal("Stopping tuntap threads - unblocking them with some self-sent data");
	while (g_running_tuntap_jobs>0) {
		_note("Sending data to unblock...");

		asio::io_service ios_local;
		auto & one_ios = ios_local;

		asio::ip::udp::socket socket_local(one_ios);
		boost::asio::ip::udp::socket & thesocket = socket_local;
		thesocket.open( asio::ip::udp::v4() );
		unsigned char data[1]; data[0]=0;
		auto buff = asio::buffer( reinterpret_cast<void*>(&data[0]), 1 );
		auto dst = asio::ip::udp::endpoint( asio::ip::address::from_string("127.0.0.1") , cfg_port_faketuntap  );
		_note("Sending to dst=" << dst);
		thesocket.send_to( buff , dst );

		std::this_thread::sleep_for( std::chrono::milliseconds(10) );
	}
	_goal("Join tuntap flow threads");
	for (auto & thr : tuntap_flow) { thr.join(); }

	_goal("Join wire ios threads");
	for (auto & thr : ios_wire_thread ) { thr.join(); }
	_goal("Join tuntap ios threads");
	for (auto & thr : ios_tuntap_thread ) { thr.join(); }
	_goal("Join general ios threads");
	{ auto & thr = ios_general_thread; thr.join(); }

	_goal("All threads done");
}

int netmodel_main(int argc, const char **argv) {
	crypto::init();
	std::vector< std::string> options;
	for (int i=1; i<argc; ++i) options.push_back(argv[i]);
	for (const string & arg : options) if ((arg=="dbg")||(arg=="debug")||(arg=="d")) g_debug = true;
	_goal("Starting program");
  asiotest_udpserv(options);
	_goal("Normal exit");
	return 0;
}

} // namespace n_netmodel

#ifdef ANTINET_PART_OF_YEDINO

#else
int main(int argc, const char **argv) {
	return n_netmodel::netmodel_main(argc,argv);
}
#endif


