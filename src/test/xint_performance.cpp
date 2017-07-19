#include "my_tests_lib.hpp"

#include "../xint.hpp"

#include <exception>
#include <cmath>
#include <type_traits>
#include <limits>

#if USE_BOOST_MULTIPRECISION

template<typename T>
size_t performance_test(const size_t m, const size_t n) {
	T **tab = new T*[m];
	for (size_t i=0; i<m; i++) {
		tab[i] = new T[n];
	}
	auto t1 = std::chrono::steady_clock::now();
	for (size_t i=0; i<m; i++) {
		for (size_t j=0; j<n; j++) {
			tab[i][j] = ((i+n)*j)%(m+n);
		}
	}
	auto t2 = std::chrono::steady_clock::now();
	for (size_t i=0; i<m; i++) {
		delete[] tab[i];
	}
	delete[] tab;
	return std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
}

TEST(xint, performance) {
	_mark("xint64 test time in ms: " << performance_test<xint64>(5000, 5000));
	_mark("int64_t test time in ms: " << performance_test<int64_t>(5000, 5000));
	_mark("xint64u test time in ms: " << performance_test<xint64u>(5000, 5000));
	_mark("uint64_t test time in ms: " << performance_test<uint64_t>(5000, 5000));
	_mark("xint32 test time in ms: " << performance_test<xint32>(5000, 5000));
	_mark("int32_t test time in ms: " << performance_test<int32_t>(5000, 5000));
	_mark("xint32u test time in ms: " << performance_test<xint32u>(5000, 5000));
	_mark("uint32_t test time in ms: " << performance_test<uint32_t>(5000, 5000));
	_mark("xint16 test time in ms: " << performance_test<xint16>(5000, 5000));
	_mark("int16_t test time in ms: " << performance_test<int16_t>(5000, 5000));
	_mark("xint16u test time in ms: " << performance_test<xint16u>(5000, 5000));
	_mark("uint16_t test time in ms: " << performance_test<uint16_t>(5000, 5000));
	_mark("xint8 test time in ms: " << performance_test<xint8>(100, 100));
	_mark("int8_t test time in ms: " << performance_test<int8_t>(100, 100));
	_mark("xint8u test time in ms: " << performance_test<xint8u>(100, 100));
	_mark("uint8_t test time in ms: " << performance_test<uint8_t>(100, 100));
}

#else

#warning "USE_BOOST_MULTIPRECISION is disabled - so we can not test this part of code"

TEST(xint, disabled) {
	TEST_IS_DISABLED("Compilation options disabled USE_BOOST_MULTIPRECISION");
}

#endif
