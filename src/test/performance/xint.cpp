#include <xint.hpp>
#include <exception>
#include <cmath>
#include <type_traits>
#include <limits>
#include <vector>

#if USE_BOOST_MULTIPRECISION

template<typename T>
size_t performance_division_test_case(const size_t m, const size_t n) {
	std::vector<std::vector<T> > vec(m);
	size_t T_max = static_cast<size_t>(std::numeric_limits<T>::max());
	for (size_t i=0; i<m; i++) {
		vec.at(i) = std::vector<T>(n);
	}
	auto t1 = std::chrono::steady_clock::now();
	for (size_t i=0; i<m; i++) {
		for (size_t j=0; j<n; j++) {
			vec[i][j] = (static_cast<T>(i%T_max)/static_cast<T>(j%T_max?j%T_max:1)/static_cast<T>(m%T_max?m%T_max:1)/static_cast<T>(n%T_max?n%T_max:1))%T_max;
		}
	}
	auto t2 = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
}

template<typename T>
size_t performance_arithmetic_test_case(const size_t m, const size_t n) {
	std::vector<std::vector<T> > vec(m);
	T T_max = std::numeric_limits<T>::max();
	T xint8_max = static_cast<T>(std::numeric_limits<xint8>::max());
	//T T_max_sqrt = std::sqrt(static_cast<double>(T_max));
	//size_t T_max_sqrt = std::sqrt(static_cast<double>(T_max));
	T T_max_sqrt = static_cast<T>(static_cast<size_t>(std::sqrt(static_cast<double>(T_max))));
	// _warn(std::sqrt(static_cast<double>(T_max)));
	const T M = m % static_cast<size_t>(T_max);
	const T N = n % static_cast<size_t>(T_max);
	for (size_t i=0; i<m; i++) {
		vec.at(i) = std::vector<T>(n, T_max);
	}
	auto t1 = std::chrono::steady_clock::now();
	for (T i=0; i<M; i++) {
		for (T j=0; j<N; j++) {
			for (T k=0; k<xint8_max; k++)
			{
				vec[i][j] = (T_max/2 + vec[i][j] % (T_max/2)) - (k % (T_max/2));
				vec[i][j] = (vec[i][j] % (T_max/2)) + (j % (T_max/2));
				vec[i][j] = vec[i][j] / (i ? i : T_max_sqrt);
				vec[i][j] = (vec[i][j] % T_max_sqrt) * (i % T_max_sqrt);
			}
		}
	}
	auto t2 = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
}

void performance_division_test() {
	_fact("[ RUN         ] xint.performance_division_test");
	auto t1 = std::chrono::steady_clock::now();
	size_t tab_m_size = 1000;
	size_t tab_n_size = tab_m_size;

	size_t xint64_time_ms =	performance_division_test_case<xint64>(tab_m_size, tab_n_size);
	size_t int64_t_time_ms = performance_division_test_case<int64_t>(tab_m_size, tab_n_size);
	_mark("xint64 is " << static_cast<double>(xint64_time_ms)/int64_t_time_ms << " times slower than int64_t");
	_note("xint64 test time in ms: " << xint64_time_ms);
	_note("int64_t test time in ms: " << int64_t_time_ms);

	size_t xint64u_time_ms = performance_division_test_case<xint64u>(tab_m_size, tab_n_size);
	size_t uint64_t_time_ms = performance_division_test_case<uint64_t>(tab_m_size, tab_n_size);
	_mark("xint64u is " << static_cast<double>(xint64u_time_ms)/uint64_t_time_ms << " times slower than uint64_t");
	_note("xint64u test time in ms: " << xint64u_time_ms);
	_note("uint64_t test time in ms: " << uint64_t_time_ms);

	size_t xint32_time_ms = performance_division_test_case<xint32>(tab_m_size, tab_n_size);
	size_t int32_t_time_ms = performance_division_test_case<int32_t>(tab_m_size, tab_n_size);
	_mark("xint32 is " << static_cast<double>(xint32_time_ms)/int32_t_time_ms << " times slower than int32_t");
	_note("xint32 test time in ms: " << xint32_time_ms);
	_note("int32_t test time in ms: " << int32_t_time_ms);

	size_t xint32u_time_ms = performance_division_test_case<xint32u>(tab_m_size, tab_n_size);
	size_t uint32_t_time_ms = performance_division_test_case<uint32_t>(tab_m_size, tab_n_size);
	_mark("xint32u is " << static_cast<double>(xint32u_time_ms)/uint32_t_time_ms << " times slower than uint32_t");
	_note("xint32u test time in ms: " << xint32u_time_ms);
	_note("uint32_t test time in ms: " << uint32_t_time_ms);

	size_t xint16_time_ms = performance_division_test_case<xint16>(tab_m_size, tab_n_size);
	size_t int16_t_time_ms = performance_division_test_case<int16_t>(tab_m_size, tab_n_size);
	_mark("xint16 is " << static_cast<double>(xint16_time_ms)/int16_t_time_ms << " times slower than int16_t");
	_note("xint16 test time in ms: " << xint16_time_ms);
	_note("int16_t test time in ms: " << int16_t_time_ms);

	size_t xint16u_time_ms = performance_division_test_case<xint16u>(tab_m_size, tab_n_size);
	size_t uint16_t_time_ms = performance_division_test_case<uint16_t>(tab_m_size, tab_n_size);
	_mark("xint16u is " << static_cast<double>(xint16u_time_ms)/uint16_t_time_ms << " times slower than uint16_t");
	_note("xint16u test time in ms: " << xint16u_time_ms);
	_note("uint16_t test time in ms: " << uint16_t_time_ms);

	size_t xint8_time_ms = performance_division_test_case<xint8>(tab_m_size, tab_n_size);
	size_t int8_t_time_ms = performance_division_test_case<int8_t>(tab_m_size, tab_n_size);
	_mark("xint8 is " << static_cast<double>(xint8_time_ms)/int8_t_time_ms << " times slower than int8_t");
	_note("xint8 test time in ms: " << xint8_time_ms);
	_note("int8_t test time in ms: " << int8_t_time_ms);

	size_t xint8u_time_ms = performance_division_test_case<xint8u>(tab_m_size, tab_n_size);
	size_t uint8_t_time_ms = performance_division_test_case<uint8_t>(tab_m_size, tab_n_size);
	_mark("xint8u is " << static_cast<double>(xint8u_time_ms)/uint8_t_time_ms << " times slower than uint8_t");
	_note("xint8u test time in ms: " << xint8u_time_ms);
	_note("uint8_t test time in ms: " << uint8_t_time_ms);

	auto t2 = std::chrono::steady_clock::now();
	auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
	_fact("[    FINISHED ] xint.performance_division_test (" << time_ms << " ms)");
}

void performance_arithmetic_test() {
	_fact("[ RUN         ] xint.performance_arithmetic_test");
	auto t1 = std::chrono::steady_clock::now();
	size_t tab_m_size = 100;
	size_t tab_n_size = tab_m_size;

	size_t xint64_time_ms =	performance_arithmetic_test_case<xint64>(tab_m_size, tab_n_size);
	size_t int64_t_time_ms = performance_arithmetic_test_case<int64_t>(tab_m_size, tab_n_size);
	_mark("xint64 is " << static_cast<double>(xint64_time_ms)/int64_t_time_ms << " times slower than int64_t");
	_note("xint64 test time in ms: " << xint64_time_ms);
	_note("int64_t test time in ms: " << int64_t_time_ms);

	size_t xint64u_time_ms = performance_arithmetic_test_case<xint64u>(tab_m_size, tab_n_size);
	size_t uint64_t_time_ms = performance_arithmetic_test_case<uint64_t>(tab_m_size, tab_n_size);
	_mark("xint64u is " << static_cast<double>(xint64u_time_ms)/uint64_t_time_ms << " times slower than uint64_t");
	_note("xint64u test time in ms: " << xint64u_time_ms);
	_note("uint64_t test time in ms: " << uint64_t_time_ms);

	size_t xint32_time_ms = performance_arithmetic_test_case<xint32>(tab_m_size, tab_n_size);
	size_t int32_t_time_ms = performance_arithmetic_test_case<int32_t>(tab_m_size, tab_n_size);
	_mark("xint32 is " << static_cast<double>(xint32_time_ms)/int32_t_time_ms << " times slower than int32_t");
	_note("xint32 test time in ms: " << xint32_time_ms);
	_note("int32_t test time in ms: " << int32_t_time_ms);

	size_t xint32u_time_ms = performance_arithmetic_test_case<xint32u>(tab_m_size, tab_n_size);
	size_t uint32_t_time_ms = performance_arithmetic_test_case<uint32_t>(tab_m_size, tab_n_size);
	_mark("xint32u is " << static_cast<double>(xint32u_time_ms)/uint32_t_time_ms << " times slower than uint32_t");
	_note("xint32u test time in ms: " << xint32u_time_ms);
	_note("uint32_t test time in ms: " << uint32_t_time_ms);

	size_t xint16_time_ms = performance_arithmetic_test_case<xint16>(tab_m_size, tab_n_size);
	size_t int16_t_time_ms = performance_arithmetic_test_case<int16_t>(tab_m_size, tab_n_size);
	_mark("xint16 is " << static_cast<double>(xint16_time_ms)/int16_t_time_ms << " times slower than int16_t");
	_note("xint16 test time in ms: " << xint16_time_ms);
	_note("int16_t test time in ms: " << int16_t_time_ms);

	size_t xint16u_time_ms = performance_arithmetic_test_case<xint16u>(tab_m_size, tab_n_size);
	size_t uint16_t_time_ms = performance_arithmetic_test_case<uint16_t>(tab_m_size, tab_n_size);
	_mark("xint16u is " << static_cast<double>(xint16u_time_ms)/uint16_t_time_ms << " times slower than uint16_t");
	_note("xint16u test time in ms: " << xint16u_time_ms);
	_note("uint16_t test time in ms: " << uint16_t_time_ms);

	size_t xint8_time_ms = performance_arithmetic_test_case<xint8>(tab_m_size, tab_n_size);
	size_t int8_t_time_ms = performance_arithmetic_test_case<int8_t>(tab_m_size, tab_n_size);
	_mark("xint8 is " << static_cast<double>(xint8_time_ms)/int8_t_time_ms << " times slower than int8_t");
	_note("xint8 test time in ms: " << xint8_time_ms);
	_note("int8_t test time in ms: " << int8_t_time_ms);

	size_t xint8u_time_ms = performance_arithmetic_test_case<xint8u>(tab_m_size, tab_n_size);
	size_t uint8_t_time_ms = performance_arithmetic_test_case<uint8_t>(tab_m_size, tab_n_size);
	_mark("xint8u is " << static_cast<double>(xint8u_time_ms)/uint8_t_time_ms << " times slower than uint8_t");
	_note("xint8u test time in ms: " << xint8u_time_ms);
	_note("uint8_t test time in ms: " << uint8_t_time_ms);

	auto t2 = std::chrono::steady_clock::now();
	auto time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
	_fact("[    FINISHED ] xint.performance_arithmetic_test (" << time_ms << " ms)");
}

int main(){
	g_dbg_level_set(40, "start performance test");
	performance_arithmetic_test();
	performance_division_test();
}

#else

#warning "USE_BOOST_MULTIPRECISION is disabled - so we can not test this part of code"

#endif
