// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#include "counter.hpp"

c_counter::c_counter(c_counter::t_count tick_len, bool is_main)
	: m_tick_len(tick_len), m_is_main(is_main),
	m_pck_all(0), m_pck_w(0), m_bytes_all(0), m_bytes_w(0)
{
	m_time_first=time(nullptr);
	m_time_ws=time(nullptr);
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
		m_time_last = std::time(nullptr);
		if (m_time_last >= m_time_ws + m_tick_len) { do_reset=1; do_print=1; }
	}
	if (do_print)	print(out);
	if (do_reset) {
		m_time_last = std::time(nullptr);
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




