
#include "libs1.hpp"

typedef string transport_id; // name next hop
typedef string hip_id; // name end src/dst

// -------------------------------------------------------------------

class c_netbuf {
	public:
		c_netbuf(size_t size);
		size_t size() const;

	private:
		vector<unsigned char> m_data;
};

c_netbuf::c_netbuf(size_t size) : m_data(size) { }

size_t c_netbuf::size() const {	return m_data.size(); }


class c_netchunk {
	public:
		c_netchunk(char * _data, size_t _size);

	public:
		char * const data; // points to inside of some existing t_netbuf. you do NOT own the data.
		const size_t size;
};

// -------------------------------------------------------------------

/**
This class wraps a network buffer and gives a circular access to it.

Writes from TUN can return up to 3 bytes (need to reserve that much always).
Writes from TUN can though return 1..3 bytes in fact (we will find after).
This number eg. 3 there is the MTU of TUN (normally e.g. 1400 or 9000 bytes or more).

How this can work, next lines are the time flow.

0123456789 <-- position. buffer size is 10

aa         push: we read 2
D F

aabb       push: we read 2
D   F

aabbc      push: we read 1
D    F

  bbc      pop:  the 2 was done (we can free it)
  D  F

  bbcddd   push: we read 3
  D     F

!!bbcddd!! push: (fail) can not guaranteed to give 3 bytes of space at end, nor at begining (*1*)
  D     F

    cddd   pop:  the 2 was done (we can free it)
    D   F

eee cddd   push: can not reserve 3bytes at end, but can at begining
   FD

eeeecddd  EXAMPLE: if we would read that much (if that would be allowed)
    X     now F==D

eee!cddd   push: (fail) can not reserve 3bytes from position

eee  ddd   pop

eee!!ddd   push: (fail) an not reserve 3bytes from position

eee        pop

eeeff      push

   ff      pop


   empty flag ?  


(*1*) we /could/ at some point allow it to return 2 buffers, usable when we

*/

class c_netbuf_circle() {
	public:
		c_netbuf_writer()

		size_t pos_F; // free_after - can write after this position
		size_t pos_D; // dirty_after - data is written after this position
		// data is free:
		// if (pos_F > pos_D)  posF .. size
		//               else  
		// data is written:

};

c_netbuf_writer::c_netbuf_writer() {
}


// -------------------------------------------------------------------

void read_from_tun(c_netbuf & entire_buf , size_t buf_) {
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

	{ // in tun-reader thread
		c_netbuf tun_inbuf( get_tun_inbuf_size() );
	}

	return 0;
}


