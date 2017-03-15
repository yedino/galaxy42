
#include "the_program_newloop.hpp"
#include "libs1.hpp"

#include "project.hpp"
#include <mo_reader.hpp>
#include "ui.hpp"
#include "trivialserialize.hpp"
#include "datastore.hpp"
#include "c_json_genconf.hpp"
#include "c_json_load.hpp"

#include "rpc/rpc.hpp"
#include "galaxy_debug.hpp"

#include "newloop.hpp"
#include <ctpl_stl.h>
#include <c_crypto.hpp>

#ifdef HTTP_DBG
	#include <thread>
	#include <mutex>
	#include <boost/asio.hpp>
	#include "httpdbg/httpdbg-server.hpp"
#endif

#include "galaxysrv.hpp" // ***

// delete this block later:
#include "cable/base/cable_base_addr.hpp"
#include "cable/base/cable_base_obj.hpp"
#include "cable/simulation/cable_simul_addr.hpp"
#include "cable/simulation/cable_simul_obj.hpp"

#include "tunserver.hpp" // delete?


// --- for using crypto directly

#include "crypto/crypto.hpp"
#include "datastore.hpp"

#include "crypto/crypto_basic.hpp"

#if ENABLE_CRYPTO_NTRU
	#include "crypto/ntrupp.hpp"

	// ntru sign
	extern "C" {
#include <constants.h>
#include <pass_types.h>
#include <hash.h>
#include <ntt.h>
#include <pass.h>
	}

#endif

#if ENABLE_CRYPTO_SIDH
	#include "crypto/sidhpp.hpp"
#endif

// ---


// ============================================================================

/// For tests this simulates the kernel that provides data that can be read via tuntap
class c_tuntap_fake_kernel {
	public:
		c_tuntap_fake_kernel();

		size_t readtun( char * buf , size_t bufsize ); ///< [thread_safe] semantics like "read" from C.

	protected:
		std::atomic<int> m_readtun_nr;
		std::vector<string> m_data; ///< example packet of data

		string make_example(int nr);
		int pseudo_rand(int r1, int r2, int r3=0);
};

c_tuntap_fake_kernel::c_tuntap_fake_kernel()
	: m_readtun_nr(0)
{
	_note("Preparing fake kernel tuntap");
	for (int nr=0; nr<4; ++nr) m_data.push_back( make_example(nr) );
}

int c_tuntap_fake_kernel::pseudo_rand(int r1, int r2, int r3) {
	long int r = r1*67481L + r2*82781L + r3*63719L;
	r1 += r1*28081L + r2*44699L;
	return (r % 104729) + (r1%8273) + (r2%9103);
}

string c_tuntap_fake_kernel::make_example(int nr) {
	string data;
	int nr_len = (nr/2)%2;
	int nr_dst = (nr/1)%2;
	int cfg_len = (vector<int>{60,100})[nr_len];
	char cfg_dst = (vector<int>{'E','I'})[nr_dst];
	for (long int i=0; i<16; ++i) data += char(i/4) + cfg_dst;
	data += '~';
	for (long int i=0; static_cast<int>(data.size())<cfg_len; ++i) {
		char c;
		if ((i%3)==0) c =  char(((i/3)%10)%10) + '0';
		if ((i%3)==1) c =  char(((i/3)%4+nr*4)%('z'-'a')) + 'A';
		if ((i%3)==2) c = '_';// char((nr%20) + 'a');
		data += c;
	}
	_info("Example data from TUN will be: " << data);
	return data;
}

size_t c_tuntap_fake_kernel::readtun( char * buf , size_t bufsize ) { // semantics like "read" from C.
	//	[thread_safe]

	int readnr = m_readtun_nr++;
	_dbg2("readtun, read#="<< readnr);
	string & this_pattern = m_data.at(readnr % m_data.size()) ;
	auto size_full = this_pattern.size();
	_check(size_full <= bufsize); // we must fit in buffer

	std::memmove( buf , this_pattern.c_str() , bufsize );

	{ // add numbers:
		size_t w = 16+1;
		int pat = readnr;
		buf[w++] = (pat >> (8*3)) % 256;
		buf[w++] = (pat >> (8*2)) % 256;
		buf[w++] = (pat >> (8*1)) % 256;
		buf[w++] = (pat >> (8*0)) % 256;
		buf[w++] = '~';

		int dp = 16+2; // data pos (in buf)
		int dl = this_pattern.size() - dp ; // data len
		_check(dl>=1);

		// std::cout << "readnr="<<readnr << " rand:" << pseudo_rand(readnr, 0) << std::endl;
		for (int i=0; i<3; ++i) buf[ pseudo_rand(readnr, i) % dl  + dp ] = 'X'+i;
	}

	return size_full;
}

// ============================================================================

class c_tuntap_fake {
	public:
		c_tuntap_fake( c_tuntap_fake_kernel & kernel );
		size_t readtun( char * buf , size_t bufsize ); ///< [thread_safe] semantics like "read" from C.
	protected:
		c_tuntap_fake_kernel & m_kernel;
};

c_tuntap_fake::c_tuntap_fake( c_tuntap_fake_kernel & kernel )
	: m_kernel( kernel )
{
	_info("Created tuntap reader, from fake kernel device at " << reinterpret_cast<void*>(&kernel) );
}

size_t c_tuntap_fake::readtun( char * buf , size_t bufsize ) {
	// [thread-safe]
	return m_kernel.readtun( buf, bufsize );
}

// ============================================================================

// -------------------------------------------------------------------

class c_the_program_newloop_pimpl {
	public:
		c_the_program_newloop_pimpl()=default;
		~c_the_program_newloop_pimpl()=default;

		unique_ptr<c_galaxysrv> server;

	private:
		friend class c_the_program_newloop;
};

// -------------------------------------------------------------------

template <typename T>
struct c_to_report {
	public:
		const T & m_obj;
		int m_level;
		c_to_report(const T & obj, int level) : m_obj(obj), m_level(level) {}
};

template <typename TS, typename TR> TS & operator<<(TS & ostr , const c_to_report<TR> & to_report) {
	to_report.m_obj.report(ostr, to_report.m_level);
	return ostr;
}

template <typename T>
const c_to_report<T> make_report(const T & obj, int level) {
	return c_to_report<T>( obj , level);
}

// -------------------------------------------------------------------

class c_netchunk {
	public:
		typedef unsigned char t_element; ///< type of one elemenet

		c_netchunk(t_element * _data, size_t _size); ///< will point to memory in data (it must be valid!) will NOT free memory
		~c_netchunk()=default; ///< does nothing (does NOT delete memory!)

		size_t size() const;
		t_element * data() const;

	public:
		t_element * const m_data; // points to inside of some existing t_netbuf. you do *NOT* own the data.
		const size_t m_size;

		void report(std::ostream & ostr, int detail) const;
};

c_netchunk::c_netchunk(t_element * _data, size_t _size) : m_data(_data), m_size(_size) { }

void c_netchunk::report(std::ostream & ostr, int detail) const {
	ostr << "netchunk this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_size
		<< ",memory@" << static_cast<const void*>(m_data) ;
	if (detail>=20) {
		ostr << " [";
		for (size_t i=0; i<m_size; ++i) {
			if (i) ostr<<' ';
			ostr << std::hex << (int)m_data[i] << std::dec ;
		}
		ostr << "]";
	}
}

size_t c_netchunk::size() const { return m_size; }
c_netchunk::t_element * c_netchunk::data() const { return m_data; }

// -------------------------------------------------------------------

/***
	@brief Gives you a buffer of continous memory of type ::t_element (octet - unsigned char) with minimal API
*/
class c_netbuf final {
	public:
		typedef c_netchunk::t_element t_element; ///< type of one elemenet

		c_netbuf(size_t size); ///< construct and allocate
		~c_netbuf(); ///< free memory

		size_t size() const;
		// vector<t_element> & get_data(); ///< access data
		// const vector<t_element> & get_data() const; ///< access data
		t_element & at(size_t ix); ///< access one element (asserted)

		t_element * data();
		t_element const * data() const;

		void report(std::ostream & ostr, int detail) const;

	private:
		t_element * m_data; ///< my actuall data storage
		size_t m_size;
};

c_netbuf::c_netbuf(size_t size) {
	_dbg2("allocating");
	m_data = new t_element[size]; // fast new - no initialization of data
	m_size = size;
	_dbg1( make_report(*this,10) );
}

c_netbuf::~c_netbuf() {
	_dbg1("dealloc: " << make_report(*this,10) );
	delete[] m_data;
	m_data=nullptr;
	m_size=0;
}

size_t c_netbuf::size() const {	return m_size; }

c_netbuf::t_element & c_netbuf::at(size_t ix) {
	_check(ix<m_size);
	return  *( m_data + ix );
}

void c_netbuf::report(std::ostream & ostr, int detail) const {
	ostr << "netBUF this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_size
		<< ",memory@" << static_cast<const void*>(m_data) ;
	if (detail>=20) {
		ostr << " [";
		for (size_t i=0; i<m_size; ++i) {
			if (i) ostr<<' ';
			ostr << std::hex << (int)m_data[i] << std::dec ;
		}
		ostr << "]";
	}
}

c_netbuf::t_element * c_netbuf::data() {	return m_data; }

c_netbuf::t_element const * c_netbuf::data() const { return m_data; }

// -------------------------------------------------------------------

// ============================================================================

c_the_program_newloop::c_the_program_newloop()
	: pimpl( new c_the_program_newloop_pimpl() )
{	}

c_the_program_newloop::~c_the_program_newloop() {
	_check( ! m_pimpl_deleted); // avoid double destruction of this object
	if (pimpl) { delete pimpl; }
	m_pimpl_deleted=true;
}

void thread_test()
{
	// thread pool example
	unsigned int number_of_threads = 4;
	auto fun = [](int id) {_note("thread id="<<id);};
	ctpl::thread_pool tp(number_of_threads);
	for(unsigned int i=0; i<10; i++) tp.push(fun);
}

///@param level - memory use allowed: level=0 - basic tests,  level=1 also SIDH, level=2 also NTru
void test_create_cryptolink(const int number_of_test, int level=0) {
	using namespace antinet_crypto;

	for (std::remove_cv<decltype(number_of_test)>::type i = 0; i < number_of_test; ++i) {
		// g_dbg_level_set(160, "start test");
		c_multikeys_PAIR keypairA, keypairB;

		keypairA.generate(e_crypto_system_type_Ed25519, 2);
		keypairB.generate(e_crypto_system_type_Ed25519, 2);

#if ENABLE_CRYPTO_SIDH
		if (level>=1) {
			keypairA.generate(e_crypto_system_type_SIDH, 2);
			keypairB.generate(e_crypto_system_type_SIDH, 2);
		}
#endif

#if ENABLE_CRYPTO_NTRU
		if (level>=2) {
			keypairA.generate(e_crypto_system_type_NTRU_sign, 1);
			keypairB.generate(e_crypto_system_type_NTRU_sign, 1);
		}
#endif

		if (level>=0) _dbg2("Advanced crypto tests enabled");

		c_multikeys_pub keypubA = keypairA.m_pub;
		c_multikeys_pub keypubB = keypairB.m_pub;

		c_crypto_tunnel AliceCT(keypairA, keypubB, "Alice");
		AliceCT.create_IDe();
		string packetstart_1 = AliceCT.get_packetstart_ab(); // A--->>>
		c_crypto_tunnel BobCT(keypairB, keypubA, packetstart_1, "Bobby");
		string packetstart_2 = BobCT.get_packetstart_final(); // B--->>>
		AliceCT.create_CTf(packetstart_2); // A<<<---

		const std::string msg(1024, 'm');
		t_crypto_nonce nonce_used;
		auto msg_encrypted = AliceCT.box(msg, nonce_used);
		_check(msg != msg_encrypted);
		auto msg_decrypted = BobCT.unbox(msg_encrypted, nonce_used);
		_check(msg == msg_decrypted);
	}
}

void c_the_program_newloop::use_options_peerref() {
	_note("Configuring my peers references (keys):");
	try {
		vector<string> peers_cmdline;
		try { peers_cmdline =m_argm["peer"].as<vector<string>>(); } catch(...) { }
		// TODO@hb no catch(...)
		for (const string & peer_ref : peers_cmdline) {
			_info( peer_ref  );
			UsePtr(pimpl->server).add_peer_simplestring( peer_ref );
		}
	} catch(const std::exception & ex) {
		ui::action_error_exit(join_string_sep( mo_file_reader::gettext("L_wrong_peer_typo") , ex.what()));
	}
}


void c_the_program_newloop::programtask_load_my_keys() {
// ------------------------------------------------------------------
// @new - now in newloop new galaxysrv
			_info("Configuring my own reference (keys):");

			bool have_keys_configured=false;
			try {
				auto keys_path = datastore::get_parent_path(e_datastore_galaxy_wallet_PRV,"");
				std::vector<std::string> keys = datastore::get_file_list(keys_path);
				size_t have_keys = (keys.size() > 0);

				auto conf_IDI = datastore::get_full_path(e_datastore_galaxy_instalation_key_conf,"IDI");
				bool conf_IDI_ok = datastore::is_file_ok(conf_IDI);

				if (have_keys) {
					if (conf_IDI_ok) {
						have_keys_configured = true;
					} else {
						_warn("You have keys, but not IDI configured. Trying to make default IDI of your keys ...");
						_warn("If this warn still occurs, make sure you have backup of your keys");
						UsePtr(pimpl->server).program_action_set_IDI("IDI");
						have_keys_configured = true;
					}
				}

			} catch(...) {
				_info("Can not load keys list or IDI configuration");
				have_keys_configured=0;
			}

			if (have_keys_configured) {
				bool ok=false;

				try {
					UsePtr(pimpl->server).configure_mykey();
					ok=true;
				} catch UI_CATCH("Loading your key");

				if (!ok) {
					_fact( "You seem to already have your hash-IP key, but I can not load it." );
					_fact( "Hint:\n"
						<< "You might want to move elsewhere current keys and create new keys (but your virtual-IP address will change!)"
						<< "Or maybe instead try other version of this program, that can load this key."
					);
					_throw_error( ui::exception_error_exit("There is existing IP-key but can not load it.") ); // <--- exit
				}
			} else {
				_fact( "You have no ID keys yet - so will create new keys for you." );

				auto step_make_default_keys = [&]()	{
					ui::action_info_ok("Generating your new keys.");
					const string IDI_name = UsePtr(pimpl->server).program_action_gen_key_simple();
					UsePtr(pimpl->server).program_action_set_IDI(IDI_name);
					ui::action_info_ok("Your new keys are created.");
					UsePtr(pimpl->server).configure_mykey();
					ui::action_info_ok("Your new keys are ready to use.");
				};
				UI_EXECUTE_OR_EXIT( step_make_default_keys );
			}
// ^ new ------------------------------------------------------------------
}




int c_the_program_newloop::main_execution() {
	_mark("newloop main_execution");
	g_dbg_level_set(5, "Debug the newloop");

	pimpl->server = make_unique<c_galaxysrv>();

	this->programtask_load_my_keys();
	this->use_options_peerref();

/*
	c_tuntap_fake_kernel kernel;
	c_tuntap_fake tuntap_reader(kernel);

	c_crypto crypto;
	std::array<unsigned char, crypto_box_NONCEBYTES> nonce;

	auto world = make_shared<c_world>();

	unique_ptr<c_cable_base_obj> cable = make_unique<c_cable_simul_obj>( world );
	unique_ptr<c_cable_base_addr> peer_addr = make_unique<c_cable_simul_addr>( world->generate_simul_cable() );

	pimpl->server = make_unique< c_galaxysrv >();


	c_netbuf buf(200);
	_note("buf: " << make_report(buf,20) );
//	for (int cycle=0; cycle<10; ++cycle) {

	if (false) {
		g_dbg_level_set(100, "Test data only");
		for(;;) {
			size_t read = tuntap_reader.readtun( reinterpret_cast<char*>( buf.data() ) , buf.size() );
			c_netchunk chunk( buf.data() , read );
			// _note("chunk: " << make_report(chunk,20) );
			std::cout << to_debug( std::string(buf.data() , buf.data()+read) , e_debug_style_buf ) << std::endl;
		}
	} else {
		for(int cycle=0; cycle<10; ++cycle) {
			size_t read = tuntap_reader.readtun( reinterpret_cast<char*>( buf.data() ) , buf.size()-crypto_box_MACBYTES);
			c_netchunk chunk( buf.data() , read );
			_note("chunk: " << make_report(chunk,20) );
			_dbg3( to_debug( std::string(buf.data() , buf.data()+read) , e_debug_style_buf ) );
			crypto.cryptobox_encrypt(chunk.data(), read, nonce, crypto.get_my_public_key());
			// TODO incr nonce
			UsePtr(cable).send_to( UsePtr(peer_addr) , chunk.data() , chunk.size() );
		}
	}

	thread_test();

	test_create_cryptolink(10,0);
*/

	_mark("newloop main_execution - DONE");

	//	newloop_main( argt );
	return 0;
}
