// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt


#pragma once
#ifndef include_counter_hpp
#define include_counter_hpp

#include "libs1.hpp"

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

	//	bool tick_restarts;
};


#endif


