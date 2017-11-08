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

#include <algorithm>

#include "../src/libs0.hpp" // libs for Antinet
#include "../src/stdplus/tab.hpp"
#include "../src/stdplus/affinity.hpp"

#include "../src-tools/tools_helper.hpp"

#include "crypto_bench/sodium_tests.hpp"

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

#define ANTINET_DEBUG_MODE 0 ///< if 1, then e.g. debug here is enabled, and some asserts/checks are executed

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


// ============================================================================

c_timerfoo g_speed_wire_recv(30); // global counter

std::atomic<bool> g_atomic_exit;
std::atomic<int> g_running_tuntap_jobs;

std::atomic<long int> g_state_tuntap2wire_started;
std::atomic<long int> g_state_tuntap_fullbuf;
c_timerfoo g_state_tuntap2wire_in_handler1(0);
c_timerfoo g_state_tuntap2wire_in_handler2(0);

// ============================================================================
// Wire
// ============================================================================

/// input buffer, e.g. for reading from wire
struct t_inbuf {
	char m_data[65535];
	// char m_data[9000];
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
		_erro("Handler hit error, ec="<<ec.message());
		return;
	}

	auto & inbuf = inbuf_tab.get(inbuf_nr);
	_note("handler for inbuf_nr="<<inbuf_nr<<" for tab at " << static_cast<void*>(&inbuf_tab)
		<< " inbuf at " << static_cast<void*>( & inbuf)
		<< " from remote IP " << inbuf.m_ep << " bytes_transferred="<<bytes_transferred
		<< " read: ["<<std::string( & inbuf.m_data[0] , bytes_transferred)<<"]"
	);

	thread_local c_timeradd tl_speed_wire_recv( g_speed_wire_recv , 1*1000 );

	if ((algo_step==e_algo_receive::after_first_read) || (algo_step==e_algo_receive::after_next_read)) {
		tl_speed_wire_recv.add(1, bytes_transferred); // [counter] inc

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
			// this function always returns 0
			// https://nacl.cr.yp.to/stream.html
			crypto_stream_xor(
				reinterpret_cast<unsigned char *>(&(inbuf.m_data[0])), // out
				reinterpret_cast<unsigned char *>(&(inbuf.m_data[0])), // bytes_transferred ])), // in XXX TODO
				bytes_transferred,
				crypto::nonce.data(), // nonce
				crypto::secret_key.data() // secret key
			);
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
						if (ec) _erro("Handler (again), size="<<bytes_transferred_again<<", ec="<<ec.message());
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
						if (ec) _erro("Handler (again), size="<<bytes_transferred_again<<", ec="<<ec.message());
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

template <typename TT>
TT assign_from_string(const std::string & s) {
	return safe_atoi(s);
}
template <>
std::string assign_from_string<std::string>(const std::string & s) {
	return s;
}

// Will set target_var with the number from "foo=42", when name="foo". Provide the cmdline;
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
			target_var = assign_from_string<T>( val );
			cmdline.m_used.at(ix) = true; // it was used
			return true;
		}
	}
	if (required) { _erro("You need to provide argument '"<<name<<"'."); throw std::runtime_error("Missing arg"); }
	return false;
}

int get_from_cmdline(const string & name, t_mycmdline &cmdline) {
	int the_val = int{};
	set_from_cmdline(the_val, name, cmdline, true);
	return the_val;
}

template <typename TT>
TT get_from_cmdline(const string & name, t_mycmdline &cmdline, TT def) {
	TT the_val = TT{};
	bool got_it = set_from_cmdline(the_val, name, cmdline, false);
	if (!got_it) the_val=def;
	return the_val;
}

// https://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
namespace xorshf96 {
	thread_local static unsigned long x=123456789, y=362436069, z=424242422;

	void init_state(long int seed) {
		auto prime1 = 2147483647 ; // https://en.wikipedia.org/wiki/2147483647
		x = x % prime1 + seed;
		y = y % prime1 + seed * 251;
		z = z % prime1 + seed * 7;
	}

	// period 2^96-1
	// MT-safe (since using thread_local globals for state)
	inline unsigned long get_value(void) {
		unsigned long t;
		x ^= x << 16;
		x ^= x >> 5;
		x ^= x << 1;
		t = x;
		x = y;
		y = z;
		z = t ^ x ^ y;
		return z;
	}
}
inline uint32_t pseudo_random() {	return xorshf96::get_value(); }

struct t_crypt_opt {
	uint32_t loops=10; ///< longer loops. E.g. value 10 bigger should be around 10 times longer and so on
	uint32_t samples=10; ///< more samples - more tests. then mediana can be calculated over them

	int modify_data=1; ///< should we modify data that we run in loop; 0=no, same data  1=modify a little

	int threads=-1; ///< how many threads to use. -1 means autodetect number of cores

	t_crypt_opt() { }
	void calculate();
};


void t_crypt_opt::calculate() {
	if (threads == -1) {
		threads = std::thread::hardware_concurrency();
	}
}

using t_bytes = std::vector<unsigned char>; ///< shortcut for: typical unsigned char network/crypto buffer, runtime size
using t_hash_random_state = char[4]; ///< using part of crypto hash (of cyphertext) as random bytes for 0-cost random generator

/// This class runs tight loop of some crypto operations, and meastures the time/speed.
/// Provide it a lambda doing main work e.g. on 3 buffers (like msg, hmac, key), and also provide same buffers to function again
/// so that it can mutate them between iterations and measure their size to calculate totall data.
/// Code inspired by sodium_tests.cpp @mikurys @rob (sodium_tests.cpp here)
template<typename F, bool allow_mt, size_t max_threads_count = 16>
class c_crypto_benchloop {
	public:
    c_crypto_benchloop(F fun);

    /// @param msg_buf - input message (e.g. in text to de/encrypt, or input msg for HMAC)
    /// @param two_buf - other buffer used depending on crypto function (decrypted/encrypted text, hmac resulting tag, or expected tag to compare with)
    /// @param key_buf - the key buffer
    /// @return speed in Gbps
		double run_test_3buf(const t_crypt_opt bench_opt, t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf);

		/// Same as run_test_3buf, but has additional argument for another key
		double run_test_4buf(const t_crypt_opt bench_opt, t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf);

	private:
    const F m_test_fun; ///< the function to be tested

		/// return speed, looking at passed time, for given amount of bytes done
		double time_finish(size_t bytes_transferred, std::chrono::time_point<std::chrono::steady_clock> time_started);

		void modify_buffers(const t_crypt_opt & bench_opt, t_hash_random_state & state,
			t_bytes & buf1, size_t size1, t_bytes & buf2, size_t size2, t_bytes & buf3, size_t size3) const;
		void modify_buffers(const t_crypt_opt & bench_opt, t_hash_random_state & state,
			t_bytes & buf1, size_t size1, t_bytes & buf2, size_t size2, t_bytes & buf3, size_t size3, t_bytes & buf4, size_t size4) const;
};

template<typename F, bool allow_mt, size_t max_threads_count>
c_crypto_benchloop<F, allow_mt, max_threads_count>
::c_crypto_benchloop(F fun)
: m_test_fun(fun)
{ }

template<typename F, bool allow_mt, size_t max_threads_count>
void c_crypto_benchloop<F, allow_mt, max_threads_count>
::modify_buffers(const t_crypt_opt & bench_opt, t_hash_random_state & state,
	t_bytes & buf1, size_t size1, t_bytes & buf2, size_t size2, t_bytes & buf3, size_t size3) const {
	modify_buffers(bench_opt,state, buf1,size1, buf2,size2, buf3,size3, buf3,0);
}

template<typename F, bool allow_mt, size_t max_threads_count>
void c_crypto_benchloop<F, allow_mt, max_threads_count>
::modify_buffers(const t_crypt_opt & bench_opt, t_hash_random_state & state,
	t_bytes & buf1, size_t size1, t_bytes & buf2, size_t size2, t_bytes & buf3, size_t size3, t_bytes & buf4, size_t size4)
	const
{
	if (bench_opt.modify_data) {
		buf1[ state[0] % size1 ] = state[1];
		buf2[ state[1] % size2 ] = state[2];
		buf3[ state[2] % size3 ] = state[3];
		if (size4) buf4[ state[3] % size4 ] = state[0];
	}
}

template<typename F, bool allow_mt, size_t max_threads_count>
double c_crypto_benchloop<F, allow_mt, max_threads_count>
::run_test_3buf(const t_crypt_opt bench_opt, t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf)
{
	t_bytes zero_buf;
	return run_test_4buf(bench_opt, msg_buf, two_buf, key_buf, zero_buf);
}

template<typename F, bool allow_mt, size_t max_threads_count>
double c_crypto_benchloop<F, allow_mt, max_threads_count>
::run_test_4buf(const t_crypt_opt bench_opt, t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf)
{
	uint32_t msg_buf_size =  msg_buf.size(), two_buf_size = two_buf.size(), key_buf_size = key_buf.size(),
		keyB_buf_size = keyB_buf.size();
	const uint32_t test_repeat = std::max<uint32_t>(50, (bench_opt.loops * 10*1000*1000) / msg_buf_size );
	auto test_repeat_point1 = (test_repeat / 5); // run that many iterations as warmup
	const auto sample_end = bench_opt.samples;
	const uint32_t worker_count = bench_opt.threads; // how many worker (threads) to use
	_note("Testing: samples: " << sample_end << " in " << worker_count << " thread(s), "
		<< "msg buf size="<<msg_buf_size<<"; Iterations per sample count: " << test_repeat << " minus warmup: " << test_repeat_point1);

	vector<double> result_sample; // results from given samples

	for (uint32_t sample=0; sample < sample_end; ++sample) {
		std::vector<double> worker_result( worker_count , {0} ); // for each worker: speed in Gbps. Each N-th worker owns N-th element so is MT-safe-for-SUCH use (only!)

		// workers notify main that they are ready:
		uint32_t worker_ready_countdown; // how many remain and are NOT yet ready
		std::condition_variable worker_ready_cv; // will notify via this cv
		std::mutex worker_ready_mu; // the lock
		// main notifies workers that it's time to start working
		bool worker_start_flag; // should worker start working now?
		std::condition_variable worker_start_cv; // will notify via this cv
		std::mutex worker_start_mu; // the lock

		std::vector<std::thread> worker_thread;

		worker_ready_countdown = worker_count; // will need this many threads to be ready before start
		worker_start_flag = false; // do not start yet

		for (uint32_t worker_nr=0; worker_nr < worker_count; ++worker_nr) {
			_dbg2("Spawning worker="<<worker_nr);
			std::thread work( [
				test_repeat_point1, test_repeat,
				msg_buf,  msg_buf_size,
				two_buf,  two_buf_size,
				key_buf,  key_buf_size,
				keyB_buf, keyB_buf_size,
				bench_opt, // read this options
				& worker_result // write result here
				// start barier:
				,worker_nr , & worker_ready_cv, & worker_ready_countdown, & worker_ready_mu,
				& worker_start_cv, & worker_start_flag, & worker_start_mu
				]( const decltype(this) my_this, uint32_t worker_nr ) // access this to call some thread-safe methods
				mutable // to modify copied buffers
				-> void
			{
				try {
					t_bytes local_msg_buf  = msg_buf;
					t_bytes local_two_buf  = two_buf;
					t_bytes local_key_buf  = key_buf;
					t_bytes local_keyB_buf = keyB_buf;
					std::chrono::time_point<std::chrono::steady_clock> local_time_started;

					t_hash_random_state hash_state; // we use results of crypot as pseudo random function - this is it's state
					std::fill_n( & hash_state[0] , 4 , 0x00);

					{ // send signall: worker -> main, that we're ready to start
						std::unique_lock<std::mutex> lg( worker_ready_mu );
						--worker_ready_countdown; // I am ready, so one less. Safe: under lock
						lg.unlock();
						worker_ready_cv.notify_one(); // tell main to check
					}
					{ // wait for main's order: worker <- main
						std::unique_lock<std::mutex> lg( worker_start_mu );
						worker_start_cv.wait(lg, [ & worker_start_flag ](){ return worker_start_flag; });
					}

					_dbg2("work#"<<worker_nr<<" starting iterations");
					for (uint32_t test_repeat_nr=0; test_repeat_nr < test_repeat; ++test_repeat_nr) {
						if (test_repeat_nr == test_repeat_point1) local_time_started = std::chrono::steady_clock::now(); // ! [timer]
						my_this->modify_buffers(bench_opt, hash_state, local_msg_buf, msg_buf_size, local_two_buf, two_buf_size, local_key_buf, key_buf_size,
							keyB_buf, keyB_buf_size);

						my_this->m_test_fun( local_msg_buf, local_two_buf, local_key_buf, local_keyB_buf); // ***

						std::copy_n( & local_two_buf[0] , 4 , & hash_state[0]); // use crypto result as random
					} // test iteration
					_dbg2("work#"<<worker_nr<<" done iterations");
					auto my_speed = my_this->time_finish( msg_buf_size * (test_repeat - test_repeat_point1) , local_time_started );
					_dbg2("work#"<<worker_nr<<" done, speed=" << my_speed);
					worker_result.at(worker_nr) = my_speed;
				}
				catch(const std::exception & ex) { _erro("Worker thread error: " << ex.what()); }
				catch(...) { _erro("Worker thread error (unknown)"); }
			} // end woker lambda
			,this , worker_nr ); // thread created
			worker_thread.push_back( std::move(work) );
		} // loop spawning workers
		_info("workers are spawned");

		// workers are now starting, some (or all) maybe even decreased worker_ready_countdown already
		{
			// waiting untill countdown is 0, checking when signalled via cv by workers
			std::unique_lock<std::mutex> lg( worker_ready_mu );
			worker_ready_cv.wait(lg, [ & worker_ready_countdown ](){ return worker_ready_countdown==0; });
		}

		// tell all workers to start now
		{
			std::unique_lock<std::mutex> lg( worker_start_mu );
			worker_start_flag = true;
			lg.unlock();
			worker_start_cv.notify_all();
		}

		_dbg1("workers started");
		for (auto & work : worker_thread) work.join(); // wait
		_dbg3("workers joined");
		double speed_avg = average( worker_result );
		result_sample.push_back( worker_count * speed_avg );
		_dbg2("sample added: " << result_sample.back() << " speed per one: " << speed_avg );
	} // sample
	return mediana( result_sample );
}


template<typename F, bool allow_mt, size_t max_threads_count>
double
c_crypto_benchloop<F, allow_mt, max_threads_count>
::time_finish(size_t bytes_transferred, std::chrono::time_point<std::chrono::steady_clock> time_started) {
	auto end_point = std::chrono::steady_clock::now(); // [timer]
	auto ellapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end_point - time_started).count() / (1000.*1000.*1000.);
	auto giga_bytes_per_second = (bytes_transferred / ellapsed) / (1000*1000*1000.);
	return giga_bytes_per_second * 8;
}

void nothing() { // TODO
	thread_local volatile bool x;
	x=1;

}

enum class e_crypto_test {
	auth_poly1305 = -10,
	veri_poly1305 = -11,
	encrypt_salsa20 = -12,
	decrypt_salsa20 = -13,
	makebox_encrypt_xsalsa20_auth_poly1305 = -14,
	openbox_decrypt_xsalsa20_auth_poly1305 = -15,
	encrypt_chacha20 = -16,
	decrypt_chacha20 = -17,
	veri_and_auth_poly1305 = -20,
	all = -100
};

void cryptotest_mesure_one(e_crypto_test crypto_op, uint32_t param_msg_size, t_crypt_opt bench_opt)
{
	const size_t msg_size = param_msg_size;
	std::vector<unsigned char> msg_buf;
	std::vector<unsigned char> two_buf;
	std::vector<unsigned char> key_buf;
	std::vector<unsigned char> keyB_buf; // the other buffer, used in eg packet forwarding (verify, auth to other key)

	size_t nonce_max_size = std::max(
		std::max( crypto_stream_salsa20_NONCEBYTES , crypto_secretbox_NONCEBYTES ) ,
		crypto_stream_chacha20_NONCEBYTES
	);
	const std::vector<unsigned char> nonce_buf( nonce_max_size , 0x00);

	auto crypto_func_auth = [msg_size](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		crypto_onetimeauth( & two_buf[0], & msg_buf[0], msg_size, & key_buf[0]);
	};
	auto crypto_func_veri = [msg_size](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		if (0 != crypto_onetimeauth_verify( & two_buf[0], & msg_buf[0], msg_size, & key_buf[0])) nothing();
	};
	auto crypto_func_veri_and_auth = [msg_size](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		if (0 != crypto_onetimeauth_verify( & two_buf[0], & msg_buf[0], msg_size, &  key_buf[0])) nothing(); // verify incoming
		// assume it was ok
		crypto_onetimeauth       ( & two_buf[0], & msg_buf[0], msg_size, & keyB_buf[0]); // auth to other peer (keyB)
	};

	// https://download.libsodium.org/doc/advanced/salsa20.html
	auto crypto_func_encr = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		crypto_stream_salsa20_xor( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0]);
	};
	auto crypto_func_decr = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) { // yeap it's identical to encrypt
		UNUSED(keyB_buf);
		crypto_stream_salsa20_xor( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0]);
	};

	// https://download.libsodium.org/doc/advanced/chacha20.html
	auto crypto_func_encr_chacha = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		crypto_stream_chacha20_xor( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0]);
	};
	auto crypto_func_decr_chacha = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) { // yeap it's identical to encrypt
		UNUSED(keyB_buf);
		crypto_stream_chacha20_xor( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0]);
	};

	// https://download.libsodium.org/doc/secret-key_cryptography/authenticated_encryption.html
	auto crypto_func_makebox = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		crypto_secretbox_easy( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0]);
	};
	auto crypto_func_openbox = [msg_size, & nonce_buf](t_bytes & msg_buf, t_bytes & two_buf, t_bytes & key_buf, t_bytes & keyB_buf) {
		UNUSED(keyB_buf);
		if (0==crypto_secretbox_open_easy( & two_buf[0], & msg_buf[0], msg_size, & nonce_buf[0], & key_buf[0])) nothing();
	};

	double speed_gbps = 0;
	std::string func_name="unknown_crypto";

	switch (crypto_op) {
		case e_crypto_test::auth_poly1305:	{
			func_name = "auth_poly1305";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(crypto_onetimeauth_BYTES, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_auth),false,1> benchloop(crypto_func_auth);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		} break;
		case e_crypto_test::veri_poly1305 : {
			func_name = "veri_poly1305";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(crypto_onetimeauth_BYTES, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_veri),false,1> benchloop(crypto_func_veri);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		} break;
		case e_crypto_test::encrypt_salsa20:	{
			func_name = "encrypt_salsa20";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(msg_size, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_encr),false,1> benchloop(crypto_func_encr);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		} break;
		case e_crypto_test::decrypt_salsa20:	{
			func_name = "decrypt_salsa20";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(msg_size, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_decr),false,1> benchloop(crypto_func_decr);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		}	break;
		case e_crypto_test::makebox_encrypt_xsalsa20_auth_poly1305:	{
			func_name = "makebox_encrypt_xsalsa20_auth_poly1305";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(msg_size + crypto_secretbox_MACBYTES, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_makebox),false,1> benchloop(crypto_func_makebox);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		} break;
		case e_crypto_test::openbox_decrypt_xsalsa20_auth_poly1305:	{
			func_name = "openbox_decrypt_xsalsa20_auth_poly1305";
			msg_buf.resize(msg_size + crypto_secretbox_MACBYTES, 0x00);
			two_buf.resize(msg_size, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_openbox),false,1> benchloop(crypto_func_openbox);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		}	break;
		case e_crypto_test::encrypt_chacha20:	{
			func_name = "encrypt_chacha20";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(msg_size, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_encr_chacha),false,1> benchloop(crypto_func_encr_chacha);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		} break;
		case e_crypto_test::decrypt_chacha20:	{
			func_name = "decrypt_chacha20";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(msg_size, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_decr_chacha),false,1> benchloop(crypto_func_decr_chacha);
			speed_gbps = benchloop.run_test_3buf(bench_opt, msg_buf, two_buf, key_buf);
		}	break;
		case e_crypto_test::veri_and_auth_poly1305:	{
			func_name = "veri_and_auth_poly1305";
			msg_buf.resize(msg_size, 0x00);
			two_buf.resize(crypto_onetimeauth_BYTES, 0x00);
			key_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			keyB_buf.resize(crypto_onetimeauth_KEYBYTES, 0xfd);
			c_crypto_benchloop<decltype(crypto_func_veri_and_auth),false,1> benchloop(crypto_func_veri_and_auth);
			speed_gbps = benchloop.run_test_4buf(bench_opt, msg_buf, two_buf, key_buf, keyB_buf);
		}	break;
		default: throw std::invalid_argument("Unknown crypto_op (enum)");
	}
	std::cout
		<< "Testing: " << func_name
		<< " msg_size_bytes: " << msg_size
		<< " Speed_in_Gbit_per_sec: " << speed_gbps
		<< " threads: " << bench_opt.threads
		<< " crypto_nr: " << static_cast<int>(crypto_op)
		<< std::endl; // output result
}

void cryptotest_main(std::vector<std::string> options) {
	_goal("Testing crypto");
	// https://download.libsodium.org/doc/advanced/poly1305.html

	t_crypt_opt bench_opt;
	const t_crypt_opt bench_opt_def; // defaults

	t_mycmdline mycmdline;
	for (const string & arg : options) mycmdline.add(arg);
	auto func_cmdline = [&mycmdline](const string &name) -> int { return get_from_cmdline(name,mycmdline); } ;
	auto func_cmdline_def = [&mycmdline](const string &name, int def) -> int { return get_from_cmdline(name,mycmdline,def); } ;
	UNUSED(func_cmdline);

	bench_opt.loops = func_cmdline_def("loops", bench_opt_def.loops);
	bench_opt.samples = func_cmdline_def("samples", bench_opt_def.samples);
	bench_opt.threads = func_cmdline_def("thr", bench_opt_def.samples);
	bench_opt.calculate(); // ***
	int opt_range_kind = func_cmdline_def("range",2); // predefined range pack
	int opt_range_one  = func_cmdline_def("rangeone",-1); // test just one value instead of testing range of values
	e_crypto_test crypto_op = static_cast<e_crypto_test>(func_cmdline_def("crypto",-10)); // crypto op, see source of cryptotest_* functions

	std::set<uint32_t> range_msgsize; // run tests on which "msgsize" parameter

	if (opt_range_one > 0) {
		opt_range_kind=0;
		range_msgsize.insert( opt_range_one );
	}

	if (opt_range_kind>=1) {
		range_msgsize.insert(64);
		range_msgsize.insert(128);
		range_msgsize.insert(512);
		range_msgsize.insert(1472);
		range_msgsize.insert(8960);
		range_msgsize.insert(140*64 *2); // 35840 = 2 jumbo
		range_msgsize.insert(140*64 *4); // 35840 = 4 jumbo
		range_msgsize.insert(65536);
	}
	if (opt_range_kind>=2) {
		for (uint32_t i=16;    i<=1500; i+=16) { auto v=i; range_msgsize.insert( v ); }
		for (uint32_t i=1500; i<=9000; i+=16) { auto v=(i/16)*16;  range_msgsize.insert( v ); }
		for (uint32_t i=9000; i<=32000; i+=128) { auto v=(i/64)*64; range_msgsize.insert( v ); }
		for (uint32_t i=32000; i<=64000; i+=1024) { auto v=(i/64)*64; range_msgsize.insert( v ); }
		for (uint32_t i=64000; i<=1024*1024*8; i+=1024*512) { auto v=(i/1024)*1024; range_msgsize.insert( v ); }
	}
	if (opt_range_kind>=3) {
		for (uint32_t i=16;    i<=1500; ++i) { auto v=i; range_msgsize.insert( v ); }
		for (uint32_t i=1500; i<=9000; i+=8) { auto v=(i/16)*16;  range_msgsize.insert( v ); }
		for (uint32_t i=9000; i<=32000; i+=128) { auto v=(i/64)*64; range_msgsize.insert( v ); }
		for (uint32_t i=32000; i<=64000; i+=256) { auto v=(i/64)*64; range_msgsize.insert( v ); }
		for (uint32_t i=64000; i<=1024*1024*8; i+=1024*128) { auto v=(i/1024)*1024; range_msgsize.insert( v ); }
	}
	if (opt_range_kind>=4) {
		for (uint32_t i=16;    i<=64000; ++i) { auto v=i; range_msgsize.insert( v ); }
	}

	std::set<int> range_threadcount; // run tests on which thread-count
	if (bench_opt.threads == -2) { // iterate on thread counts
		int max_threads = std::thread::hardware_concurrency() * 2;
		for (int t=1; t<max_threads; ++t) range_threadcount.insert(t);
	}
	else range_threadcount.insert( bench_opt.threads );

	std::set<e_crypto_test> range_crypto_op; // which crypto tests to run
	if (crypto_op == e_crypto_test::all) { // special case - many crypto_op to test
		// TODO: foreach enum
		range_crypto_op.insert(e_crypto_test::auth_poly1305);
		range_crypto_op.insert(e_crypto_test::encrypt_salsa20);
		range_crypto_op.insert(e_crypto_test::decrypt_salsa20);
		range_crypto_op.insert(e_crypto_test::makebox_encrypt_xsalsa20_auth_poly1305);
		range_crypto_op.insert(e_crypto_test::openbox_decrypt_xsalsa20_auth_poly1305);
		range_crypto_op.insert(e_crypto_test::encrypt_chacha20);
		range_crypto_op.insert(e_crypto_test::decrypt_chacha20);
		range_crypto_op.insert(e_crypto_test::veri_and_auth_poly1305);
	}
	else range_crypto_op.insert( crypto_op ); // one op given by it's number

	_goal("Will test various msg-size, count: " << range_msgsize.size() );
	_goal("Will test various thread-count, count: " << range_threadcount.size() );
	_goal("Will test various crypto_op, count: " << range_crypto_op.size() );

	for(auto thr : range_threadcount) {
		_clue("For thread count: " << thr);
		auto bench_this_one = bench_opt;
		bench_this_one.threads = thr;
		for(auto crypto_op_current : range_crypto_op) {
			for(auto msg_size : range_msgsize) cryptotest_mesure_one(crypto_op_current, msg_size, bench_this_one);
		}
	}
}

void asiotest_udpserv(std::vector<std::string> options) {
	_goal("Starting " << __func__ << " with " << options.size() << " arguments");
	for (const auto & arg: options) _note("Arg: ["<<arg<<"]");
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

	const vector<int> cfg_wire_ios_cpu // [4] = 7 would mean: for wire ios index #4, run it on cpu number #7
	=
	[&mycmdline]() {
		vector<int> ret;
		string choice = get_from_cmdline("wire_cpu", mycmdline, std::string(""));
		_mark("cpu configuration: \"" << choice << "\"");
		auto the_size = choice.size();
		if (the_size<1) return ret;
		// --- size>1 ---
		size_t pos1=0;
		while (true) {
			// _mark("parse, pos1="<<pos1);
			size_t pos2 = choice.find(',', pos1);
			if (pos2 == string::npos) pos2=choice.size();
			// _mark("parse, pos2="<<pos2);

			string cpu_str = choice.substr(pos1,pos2-pos1);
			// _mark("cpu_str=\""<<cpu_str<<"\"");
			ret.push_back( safe_atoi( cpu_str ) );
			if ( pos2 >= (the_size-1) ) break;
			pos1=pos2+1;
			// _mark(cpu_str);
		}
		return ret;
	} ();

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

	const int cfg_run_timeout = func_cmdline_def("run_timeout", 30);

	vector<asio::ip::udp::endpoint> peer_pegs;
	//peer_pegs.emplace_back( asio::ip::address_v4::from_string("127.0.0.1") , 9000 );
	_note("Adding peer");
	{
		auto opt_addr = options.at(0);
		auto opt_port = options.at(1);
		_goal("Peer address ["<<opt_addr<<"] and prot ["<<opt_port<<"]");
		peer_pegs.emplace_back(
			asio::ip::address_v4::from_string(opt_addr) ,
			safe_atoi(opt_port));
	}
	_goal("Got peer(s) " << peer_pegs.size());

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
	}

	_note("Create ios (TUNTAP)");
	std::vector<std::unique_ptr<asio::io_service>> ios_tuntap;
	std::vector<std::unique_ptr<asio::io_service::work>> ios_tuntap_work;
	for (int i=0; i<cfg_tuntap_ios; ++i) {
		_goal("Creating ios (TUNTAP) nr "<<i);
		ios_tuntap.emplace_back( std::make_unique<asio::io_service>() );
		ios_tuntap_work.emplace_back( std::make_unique<asio::io_service::work>( * ios_tuntap.back() ) );
	}

	boost::asio::signal_set signals( ios_general, SIGINT);
	signals.async_wait( handler_signal_term );
	std::thread ios_general_thread([&ios_general](){
		ios_general.run();
	});

	vector<std::thread> ios_wire_thread;
	for (int ios_nr = 0; ios_nr < cfg_num_ios; ++ios_nr) {
		for (int ios_thread=0; ios_thread<cfg_num_thread_per_ios; ++ios_thread) {
			int cpu_nr = container_get_or_default( cfg_wire_ios_cpu , ios_thread , -1 );
			_goal("WIRE: start worker: ios_nr=" << ios_nr << " ios_thread=" << ios_thread
				<< " cpu=" << cpu_nr << " --- cpu ---");

			std::thread thread_run(
				[&ios_wire, ios_thread, ios_nr, cpu_nr] {

					try {
						stdplus::affinity::set_current_thread_affinity( cpu_nr );
					}
					catch (const std::exception & ex) {
						_erro("Can not set CPU: " << ex.what());
					}

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
	_goal("The stop (and stats) thread"); // exit flag --> ios.stop()
	std::thread thread_stop(
		[&ios_general,&ios_wire,&ios_tuntap, &ios_general_work, &ios_wire_work, &ios_tuntap_work, &welds, &welds_mutex, &cfg_run_timeout] {

			std::vector<double> speed_tab;

			auto run_time_start = std::chrono::steady_clock::now();

			for (long int sample=0; true; ++sample) {
				std::this_thread::sleep_for( std::chrono::milliseconds(500) );

				auto run_time_now = std::chrono::steady_clock::now();
				int run_time_ellapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(run_time_start - run_time_now).count();
				if ( run_time_ellapsed_sec > cfg_run_timeout) {
					_mark("Test will end now, time allapsed: " << run_time_ellapsed_sec);
					g_atomic_exit = true;
				}

				g_speed_wire_recv.step();
//				g_state_tuntap2wire_in_handler1.step();
//				g_state_tuntap2wire_in_handler2.step();

				// [counter] read
				std::ostringstream oss;
				oss << "Loop. ";
				oss << "Wire: RECV={" << g_speed_wire_recv << "}";
				oss << "; ";
				oss << "Tuntap: ";
				oss << "start="<<g_state_tuntap2wire_started.load(std::memory_order_relaxed)<<' ';
//				oss << "h1={"<<g_state_tuntap2wire_in_handler1<<"} ";
//				oss <<" h2={"<<g_state_tuntap2wire_in_handler2<<"} ";
				oss <<" fullBuf="<<g_state_tuntap_fullbuf.load(std::memory_order_relaxed)<<" ";
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

		auto addr_listen = asio::ip::address::from_string("0.0.0.0");
		// asio::ip::address_v4::any();
		// if (nr_sock==0) addr_listen = asio::ip::address::from_string("192.168.113.16");
		// if (nr_sock==1) addr_listen = asio::ip::address::from_string("192.168.1.102");
		_mark("Using special addressing (TEST!)"); // XXX TODO
		_mark("Listen on: " << addr_listen);

		thesocket.open( asio::ip::udp::v4() );
		// thesocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		thesocket.bind( asio::ip::udp::endpoint( addr_listen , port_nr ) );
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
		_note("Creating workflow (blocking - thread) for tuntap, socket="<<tuntap_socket_nr);

		std::thread thr = std::thread(
			[tuntap_socket_nr, &tuntap_socket, &welds, &welds_mutex, &wire_socket, &peer_pegs, cfg_tuntap_buf_sleep, cfg_size_tuntap_maxread]()
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
					g_state_tuntap2wire_started.fetch_add(std::memory_order_relaxed);

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
							g_state_tuntap_fullbuf.fetch_add(std::memory_order_relaxed);
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
		_goal("Creating workflow: buf="<<inbuf_nr<<" socket="<<socket_nr);

		auto inbuf_asio = asio::buffer( inbuf_tab.addr(inbuf_nr) , t_inbuf::size() );
		_dbg1("buffer size is: " << asio::buffer_size( inbuf_asio ) );
		_dbg1("async read, on mysocket="<<addrvoid(wire_socket));
		{
			// std::lock_guard< std::mutex > lg( mutex_handlerflow_socket ); // LOCK

			auto & this_socket_and_strand = wire_socket.at(socket_nr);

			// [asioflow]
			this_socket_and_strand.get_strand().post([&inbuf_tab, &mutex_handlerflow_socket_wire,   &this_socket_and_strand, inbuf_nr, inbuf_asio] {
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
	_goal("Entering the network model");
	crypto::init();
	std::vector< std::string> options;
	for (int i=1; i<argc; ++i) options.push_back(argv[i]);
	enum class t_testmode { e_testmode_net, e_testmode_crypto } testmode = t_testmode::e_testmode_net;
	for (const string & arg : options) if ((arg=="dbg")||(arg=="debug")||(arg=="d")) g_debug = true;
	for (const string & arg : options) if ((arg=="onlycrypto")) testmode = t_testmode::e_testmode_crypto;
	_goal("Starting netmodel");
	switch (testmode) {
		case t_testmode::e_testmode_net:
			asiotest_udpserv(options);
		break;
		case t_testmode::e_testmode_crypto:
			cryptotest_main(options);
		break;
	}

	_goal("Normal exit of netmodel");
	return 0;
}

} // namespace n_netmodel

#ifdef ANTINET_PART_OF_YEDINO

#else
int main(int argc, const char **argv) {
	_goal("Starting the network model tool (stand alone program)");
	std::srand( time(nullptr) );
	return n_netmodel::netmodel_main(argc,argv);
}
#endif
