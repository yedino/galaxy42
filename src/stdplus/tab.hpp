// Copyrighted (C) 2015-2017 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once

/**
@file stdplus is library of very common functions to be used in this and other projects,
stdplus/tab focuses on containers support
@owner rfree
@todo TODO@testcase - test cases for all functions here
*/

#include <memory>
#include <utils/check.hpp>
#include <utils/misc.hpp>


namespace stdplus {


/*
Copies n elements from tab_src container (starting from offset_src)
into tab_dst table (starting in it at index offset_dst).
Both containers must be apart (that is: they must not overlap  at least in the range being copied).
Both containers must be SequenceContainer and also ContiguousIterator(as defined for C++17) - e.g. std::vector<> will work.
If conditions are not meet, then function at most throws exception (hard _check error),
how ever it can also do nothing if given invalid condition was not needed anyway (e.g. for n==0).
*/
template <class T1, class T2>
typename T2::iterator copy_safe_apart(size_t n, const T1 & tab_src, T2 & tab_dst, size_t offset_src=0, size_t offset_dst=0) {
	_dbg4("copy (tab) n="<<n);
	if (n<1) return tab_dst.begin();

	// source
	auto src_rb = tab_src.begin() + offset_src; // rb = range begin
	_check_input( offset_src < tab_src.size() );
	_check_input( n <= tab_src.size() - offset_src ); // subtracting offset_src is valid since above
	auto src_rl = tab_src.begin() + offset_src + n -1; // range last
	_dbg4("Source     range src_rb="<<to_debug(src_rb)<<" ... src_rl="<<to_debug(src_rl));
	_check_abort( src_rl <= tab_src.end() );

	// dest
	auto dst_rb = tab_dst.begin() + offset_dst; // rb = range begin
	_check_input( offset_dst < tab_dst.size() );
	_check_input( n <= tab_dst.size() - offset_dst ); // subtracting offset_dst is valid since above
	auto dst_rl = tab_dst.begin() + offset_dst + n -1; // range last
	_dbg4("Destintion range dst_rb="<<to_debug(dst_rb)<<" ... dst_rl="<<to_debug(dst_rl));
	_check_abort( dst_rl <= tab_dst.end() );

	bool overlap = test_ranges_overlap_notempty_asserted(src_rb, src_rl,  dst_rb, dst_rl);
	_dbg4("overlap=" << overlap);
	_check_input(!overlap);

	copy_and_assert_no_overlap_size( src_rb, src_rl,  dst_rb, n );
	_dbg4("Copy done.");

	return dst_rb;
}





} // namespace stdplus



