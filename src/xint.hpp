#pragma once

 #include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
xint;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void> >
uxint;

