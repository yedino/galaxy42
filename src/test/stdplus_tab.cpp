#include "gtest/gtest.h"

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <string>
#include <array>

#include <stdplus/tab.hpp>

template <class TAB1, class TAB2>
void testcase_vectorlike() {
	TAB1 tab1;
	tab1.push_back(42);
	tab1.push_back(100);
	TAB2 tab2;
	tab2.resize(2);

	stdplus::copy_safe_apart(2, tab1, tab2);
	auto it1=tab1.begin();
	auto it2=tab2.begin();
	for (size_t i=0; i<2; ++i) {
		EXPECT_EQ( *it1, *it2 );
		++it1;
		++it2;
	}
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



TEST(stdplus_tab, basicuse) {
	// testcase< std::vector<int> , std::vector<int> > ( ) ;
	testcase_vectorlike<  std::vector<int> , std::vector<int>  > ( ) ;

	std::array<int,2> arr1,arr2;
	testcase_arraylike(arr1,arr2);
}

