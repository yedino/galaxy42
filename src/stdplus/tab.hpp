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

/**
 * Check if elements of container are stored contiguously
 */
template <typename T>
struct is_stl_container_contiguously : std::false_type {
};

template <typename T, typename A>
struct is_stl_container_contiguously<std::vector<T, A>> : std::true_type {
};

template <typename T, size_t N>
struct is_stl_container_contiguously<std::array<T, N>> : std::true_type {
};

template <typename T>
struct is_stl_container_contiguously<std::basic_string<T>> : std::true_type {
};

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
	static_assert(is_stl_container_contiguously<T1>::value, "Template parameter T1 is not stl contiguously container");
	static_assert(is_stl_container_contiguously<T2>::value, "Template parameter T2 is not stl contiguously container");
	if (n<1) return tab_dst.begin();

	// source
	auto src_rb = tab_src.begin() + offset_src; // rb = range begin
	_check_input( offset_src < tab_src.size() );
	_check_input( n <= tab_src.size() - offset_src ); // subtracting offset_src is valid since above
	auto src_re = tab_src.begin() + offset_src + n; // range end
	_check_abort( src_re <= tab_src.end() );

	// dest
	auto dst_rb = tab_dst.begin() + offset_dst; // rb = range begin
	_check_input( offset_dst < tab_dst.size() );
	_check_input( n <= tab_dst.size() - offset_dst ); // subtracting offset_dst is valid since above
	auto dst_re = tab_dst.begin() + offset_dst + n; // range end
	_check_abort( dst_re <= tab_dst.end() );

	bool overlap = ranges_overlap_oc_ne_asserted(src_rb, src_re,  dst_rb, dst_re);
	_check_input(!overlap);

	copy_and_assert_no_overlap_size( src_rb, src_re,  dst_rb, n );

	return dst_rb;
}





} // namespace stdplus



