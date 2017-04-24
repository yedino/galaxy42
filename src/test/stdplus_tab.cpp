#include "gtest/gtest.h"

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include <array>

#include <stdplus/tab.hpp>
#include <stdplus/misc.hpp>

TEST(stdplus_tab, helper_overlap) {

	std::vector<int> aaa{100, 101, 102, 103, 104};
	std::vector<int> bbb{200, 201, 202, 203, 204};

	EXPECT_EQ( true  , test_ranges_overlap_inclusive_noempty(aaa.begin()+0,aaa.begin()+3, aaa.begin()+3, aaa.begin()+4) );
	EXPECT_EQ( false , test_ranges_overlap_inclusive_noempty(aaa.begin()+0,aaa.begin()+3, aaa.begin()+4, aaa.begin()+4) );

}

template <class TAB1, class TAB2>
void testcase_vectorlike_copyall(size_t n) {
	_info("Test for n="<<n);
	TAB1 tab1;
	for (size_t i=0; i<n; ++i) tab1.push_back(1000+i);

	TAB2 tab2;
	tab2.resize(n);

	stdplus::copy_safe_apart(n, tab1, tab2);
	auto it1=tab1.begin();
	auto it2=tab2.begin();
	for (size_t i=0; i<n; ++i) {
		EXPECT_EQ( *it1, *it2 );
		++it1;
		++it2;
	}
	_info("Test for n="<<n<<" done ");
}

template <class TAB1, class TAB2>
void testcase_vectorlike_offset2(size_t size1, size_t size2, size_t n, size_t off1, size_t off2) {
	_info("Test size1="<<std::setw(4)<<size1<<" size2="<<size2<<" n="<<n<<" off1="<<off1<<" off2="<<off2);
	TAB1 tab1;
	typedef typename TAB1::value_type t_value;
	const t_value test1=1000, test2=2000; // test values
	for (size_t i=0; i<size1; ++i) { t_value iv = static_cast<t_value>(i);  tab1.push_back(test1+iv); }

	TAB2 tab2;
	for (size_t i=0; i<size2; ++i) { t_value iv = static_cast<t_value>(i);  tab2.push_back(test2+iv); }

	stdplus::copy_safe_apart(n, tab1, tab2, off1, off2);
	for (size_t i=0; i<size1; ++i) { t_value iv = static_cast<t_value>(i);  EXPECT_EQ( tab1.at(i) , test1+iv ); } // source is unchanged
	for (size_t i=0; i<off2; ++i) { t_value iv = static_cast<t_value>(i);  EXPECT_EQ( tab2.at(i) , test2+iv ); } // dest part before - is unchanged
	for (size_t i=off2; i<n; ++i) { t_value iv = static_cast<t_value>(i);  EXPECT_EQ( tab2.at(i) , tab1.at(i-off2+off1) );  UNUSED(iv); } // dest overwritten
	for (size_t i=off2+n; i<size2; ++i) { t_value iv = static_cast<t_value>(i);  EXPECT_EQ( tab2.at(i) , test2+iv ); } // dest part after - is unchanged
}

template <class TAB1, class TAB2>
void testcase_arraylike(const TAB1& tab1, TAB2& tab2) {
	stdplus::copy_safe_apart(2, tab1, tab2);
	auto it1=tab1.begin();
	auto it2=tab2.begin();
	for (size_t i=0; i<2; ++i) {
		EXPECT_EQ( *it1, *it2 );
		++it1;
		++it2;
	}
}


template<class T>
T fuzzy_plus_minus(typename T::value_type n, const T & tab) {
	T ret;
	for (const auto v: tab) {
		for (long long int i=-n; i<+n; i++) {
			ret.insert( v+i );
		}
	}
	return ret;
}

TEST(stdplus_tab, simple) {
	g_dbg_level_set(5,"test");
	std::array<int,2> arr1,arr2;
	stdplus::copy_safe_apart(2,arr1,arr2,0,0);
}



TEST(stdplus_tab, basicuse) {
	g_dbg_level_set(5,"test");
	// testcase< std::vector<int> , std::vector<int> > ( ) ;
	testcase_vectorlike_copyall<  std::vector<int> , std::vector<int>  > ( 10 ) ;

	std::array<int,2> arr1,arr2;
	testcase_arraylike(arr1,arr2);
}


template <class TTab1, class TTab2>
void tests_loop() {
	size_t test_count=0;
	for (const auto size1 : std::set<long int>{ 0, 1, 2, 3, 4, 5, 10 }) {
		for (const auto size2 : fuzzy_plus_minus(2, std::set<long int>{ 0, 1, 2, 3, 4, 5, 10, size1/2, size1 } ) ) {
			if (! (size2>=0)) continue;
			for (const auto num : fuzzy_plus_minus(2, std::set<long int>{ 0, 1, 5, 10, size1/2, size1 } ) ) {
				if (! (num>=0)) continue;
				for (const auto off1 : fuzzy_plus_minus(2, std::set<long int>{ 0, 1, 5, 10, size1/2, size1 } ) ) {
					if (! (off1>=0)) continue;
					if (! ((num+off1)<=size1)) continue; // we use a valid range inside source container
					for (const auto off2 : fuzzy_plus_minus(2, std::set<long int>{ 0, 1, 5, 10, size1/2, size1, off1, off1/2 } ) ) {
						if (! (off2>=0)) continue;
						if (! ((num+off2)<=size2)) continue; // we use a valid range inside destination container
						++test_count;
						testcase_vectorlike_offset2<  TTab1 , TTab2  > ( size1 , size2, num,  off1 , off2 ) ;
					}
				}
			}
		}
	}
	_fact("Executed tests count: " << test_count );
}

TEST(stdplus_tab, loop_vector) {
	tests_loop< std::vector<int> , std::vector<int> >();
	// tests_loop< std::vector<int> , std::vector<long int> >(); // TODO@rfree
}

