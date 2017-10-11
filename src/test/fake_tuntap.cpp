#include "fake_tuntap.hpp"

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

std::string c_tuntap_fake_kernel::make_example(int nr) {
	std::string data;
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
	std::string & this_pattern = m_data.at(readnr % m_data.size()) ;
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

c_tuntap_fake::c_tuntap_fake( c_tuntap_fake_kernel & kernel )
	: m_kernel( kernel )
{
	_info("Created tuntap reader, from fake kernel device at " << reinterpret_cast<void*>(&kernel) );
}

size_t c_tuntap_fake::readtun( char * buf , size_t bufsize ) {
	// [thread-safe]
	return m_kernel.readtun( buf, bufsize );
}
