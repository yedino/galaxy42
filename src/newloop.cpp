#include "libs1.hpp"
#include "utils/check.hpp"

typedef string transport_id; // name next hop
typedef string hip_id; // name end src/dst


void	image(int size) {
	_info("image size="<<size);
	_try_user(size < 100);

	auto size2 = size * size;
	_check_user( size2 < 1000 );
	_check_user( size2 >= 4 );
	auto mem1 = new int[ size ];
	auto mem2 = new int[ size2 ];
	_dbg1( (void*)mem1 << " " << (void*)mem2 << " " << size2 );
}

void connect_peer(bool good_version, bool good_data) {
	try {
		_try_extern(good_version == true);
		_check_extern(good_data == false);
	}
	catch(err_check_user & ex) { _erro("User (not-fixed):" << ex.what()); }
	catch(err_check_base & ex) { _erro("Check (not-fixed):" << ex.what()); }
	catch(std::runtime_error & ex) { _erro("Runtime:" << ex.what()); }
	catch(std::bad_alloc & ex) { _erro("Bad alloc:" << ex.what()); }
	catch(std::exception & ex) { _erro("Exception:" << ex.what()); }
	catch(...) { _erro("Exception of unknown type"); }
}

void image_ui(int size, string dir="/tmp", string fname="/tmp/data") {
	_info("For size=" << size);
	try {
		bool done=false; int trynr=0;
		while (!done) {
			try {
				image(size);
				_try_sys( dir=="/tmp" ); // peretending here we actually stat() the directory here
				_check_sys( fname=="/tmp/data" ); // pretending we here read the file and if it's other file then it is invalid
				done=true;
			}
			catch(err_check_soft & ex) {
				_warn("Soft:" << ex.what());
				++trynr;
				if (trynr>3) throw;
				_note("Will retry...");
				if (size>20) size -= 5; // maybe the size is too big, try to fix that
			}
		}
	}
	catch(err_check_user & ex) { _erro("User (not-fixed):" << ex.what()); }
	catch(err_check_base & ex) { _erro("Check (not-fixed):" << ex.what()); }
	catch(std::runtime_error & ex) { _erro("Runtime:" << ex.what()); }
	catch(std::bad_alloc & ex) { _erro("Bad alloc:" << ex.what()); }
	catch(std::exception & ex) { _erro("Exception:" << ex.what()); }
	catch(...) { _erro("Exception of unknown type"); }
}

void test_debug_check() {
	image_ui(10); // ok
	image_ui(101); // ok (after retry)

	// regarding user errors:
	image_ui(120); // soft
	image_ui(70); // hard - unexpected, anything above 30 is bad on size*size condition
	image_ui(1); // hard - unexpected small size*size
	_mark("More tests");

	// regarding system errors:
	image_ui(10,"/mount/z"); // soft system
	image_ui(10,"/tmp", "/tmp/badfile"); // hard system

	connect_peer(true, true); // ok
	connect_peer(false,true); // soft
	connect_peer(true, false); // hard

	image_ui(-3); // bad alloc
}

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

		c_netchunk(t_element * _data, size_t _size);

	public:
		t_element * const data; // points to inside of some existing t_netbuf. you do *NOT* own the data.
		const size_t size;
};

// -------------------------------------------------------------------

/***
	@brief Gives you a buffer of continous memory of type ::t_element (octet - unsigned char) with minimal API
*/
class c_netbuf {
	public:
		typedef c_netchunk::t_element t_element; ///< type of one elemenet

		c_netbuf(size_t size); ///< construct and allocate

		size_t size() const;
		// vector<t_element> & get_data(); ///< access data
		// const vector<t_element> & get_data() const; ///< access data
		t_element & at(size_t ix); ///< access one element (asserted)

		void report(std::ostream & ostr, int detail) const;

	private:
		vector<unsigned char> m_data; ///< my actuall data storage
};

c_netbuf::c_netbuf(size_t size) : m_data(size) {
	_dbg1( make_report(*this,10) );
}

size_t c_netbuf::size() const {	return m_data.size(); }

c_netbuf::t_element & c_netbuf::at(size_t ix) { return m_data.at(ix); }

void c_netbuf::report(std::ostream & ostr, int detail) const {
	ostr << "this@" << static_cast<const void*>(this);
	if (detail>=1) ostr << " m_data@" << static_cast<const void*>(this) << ",size=" << m_data.size()
		<< ",memory@" << static_cast<const void*>(m_data.data()) ;
}


// -------------------------------------------------------------------

/**
This class wraps a network buffer and gives a circular access to it.

Writes from TUN can return up to 3 bytes (need to reserve that much always).
Writes from TUN can though return 1..3 bytes in fact (we will find after).
This number eg. 3 there is the MTU of TUN (normally e.g. 1400 or 9000 bytes or more).

How this can work, next lines are the time flow:

0123456789 <-- position. buffer size is 10, this are collumns for all lines below, showing data in buff.

aa         push: we read 2. Our buffer have in it "aa" from position 0 to position 1 inclusive.
D>         D=buf[0], S=2

aabb       push: we read 2
D-->       D=buf[0], S=4

aabbc      push: we read 1
D--->

  bbc      pop:  the 2 was done (we can free it)
  D->      D=buf[2], S=3

  bbcddd   push: we read 3
  D    F

!!bbcddd!! push: (fail) can not guaranteed to give 3 bytes of space at end, nor at begining (*1*)
  D    F

    cddd   pop:  the 2 was done (we can free it)
    D  F

eee cddd   push: can not reserve 3bytes at end, but can at begining
  F D

eeeecddd  EXAMPLE: if we would read that much (if that would be allowed)
   FD     now F==D

eee!cddd   push: (fail) can not reserve 3bytes from position

eee  ddd   pop

eee!!ddd   push: (fail) an not reserve 3bytes from position

eee        pop

eeeff      push

   ff      pop


Expressing position:


.......... S=0. D can be anything, e.g. D=buf[0]

a......... D=buf[0], S=0
D

.abcdefghi
 D------->

abcdefghi.
D------->

abcdefghij
D-------->

hijabcdefg
-->D------

cccaaabb..
==>D--->

bbbc.aaa..
bbbc......
bbbcddd...
...cddd...


0123456789


(*1*) we /could/ at some point allow it to return 2 buffers, usable when we

*/

class c_netbuf_circle {
	public:
		c_netbuf_circle();

		size_t pos_D; // position of data start.
		size_t size_from_D; // size of data allocated after pos_D.
		size_t size_from_0; // size of data allocated in addition from start. Normalize it: prefer this to be 0 when possible.

		vector<char> dbg_data_name; ///< for debug only: vector representing this memory, with it's 1-character printable name
		vector<long int> dbg_data_id; ///< for debug only: vector representing this memory, with unique ID for unit-tests etc
};

c_netbuf_circle::c_netbuf_circle() {
}


// -------------------------------------------------------------------

void read_from_tun(c_netbuf & entire_buf , size_t buf_) {
	_UNUSED( entire_buf );
	_UNUSED( buf_ );
}

int get_tun_inbuf_size() { // the inbuffer used to read from TUN; each tun-reader thread has own inbuf.
	return 9000;
}

int get_mts(transport_id t) { // maximum transport size
	if (t=="A") return 4;
	return 8;
}



// ideas:
// http://stackoverflow.com/questions/15219984/using-vectorchar-as-a-buffer-without-initializing-it-on-resize/15220853#15220853

int newloop_main(int argc, const char **argv) {
	_UNUSED(argc);
	_UNUSED(argv);

	g_dbg_level_set(10, "Debug the newloop");

	test_debug_check();

	{ // in tun-reader thread
		c_netbuf tun_inbuf( get_tun_inbuf_size() );
	}

	return 0;
}


