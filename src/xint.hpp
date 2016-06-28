#pragma once

#include <iostream>
#include <ostream>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_int/limits.hpp>

#include "libs0.hpp"

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
basic_xint;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void> >
basic_uxint;


typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<128, 1024,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
xbigint;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<128, 1024,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void> >
uxbigint;



typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<256, 4096,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
t_correct_int;

template<typename T>
using t_int_representing_float =
	boost::multiprecision::number<
		boost::multiprecision::cpp_int_backend<
			std::numeric_limits<T>::max_exponent,
			std::numeric_limits<T>::max_exponent,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void>
	>
;


// TODO move to lib
// idea from http://stackoverflow.com/questions/16337610/how-to-know-if-a-type-is-a-specialization-of-stdvector
template <typename T, template<typename...> class R>
struct is_specialization : std::false_type { };
template <template<typename...> class R, typename... ARGS>
struct is_specialization<R<ARGS...>, R> : std::true_type { };



template <typename T, typename U>
bool overflow_impossible_in_assign(T target, U value) {
	if (value < std::numeric_limits<decltype(target)>::min()) return false;
	if (value > std::numeric_limits<decltype(target)>::max()) return false;
	return true; // no overflow
}

struct safer_int_base { };


template<typename T>
class safer_int : public safer_int_base {
		#define TEMPLATE \
		template<\
			unsigned U1, unsigned U2, \
			boost::multiprecision::cpp_integer_type U3, boost::multiprecision::cpp_int_check_type U4, \
			typename U5, boost::multiprecision::expression_template_option U6>
		#define T_OBJECT boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<U1, U2, U3, U4, U5>, U6>

	public: // <-- TODO private
		T xi; ///< the xint implementig my basic type
	public:
		/// construct - default
		safer_int() : xi( T() )	{	}

	//	explicit safer_int(const char* txt) : xi(txt) { } // TODO check overflow, does result match txt

		// construct from same type
		template<typename U> safer_int(const safer_int<U> & obj)
			: xi(obj.xi)
		{
			// TODO numeric_cast
		}

		/// construct from various "int" types
		template<typename U, typename std::enable_if<
			std::is_integral<U>{} && std::is_fundamental<U>{}  >::type* = nullptr>
		safer_int(U obj) : xi(obj)
		{
			// _warn("Creating from INT, type=" << typeid(U).name() << " value=" << obj );
			// TODO numeric_cast
		}

		/// construct from various "float" types
		template<typename U, typename std::enable_if<
			std::is_arithmetic<U>{} && ! std::is_integral<U>{} && std::is_fundamental<U>{}  >::type* = nullptr>
		safer_int(U obj)
		{
			_warn("Creating from float, type=" << typeid(U).name() << " value=" << obj );

			t_int_representing_float<U> obj_as_xi( obj );
			if (!overflow_impossible_in_assign(xi,obj_as_xi)) throw boost::numeric::bad_numeric_cast();
			xi = static_cast<T>(obj);
		}

		/// construct from same boost multiprecision number
		TEMPLATE
		safer_int(T_OBJECT obj)
			: xi(obj)
		{ }

		template<typename U> safer_int<T> & operator=(const safer_int<U> & obj) {
			xi=obj.xi;
			// TODO numeric_cast
			return *this;
		}
		//template<typename U> safer_int<T> & operator=(U obj) {
		safer_int<T> & operator=(int64_t obj) {
			xi=obj;
			// TODO numeric_cast
			return *this;
		}

		template<typename U> operator safer_int<U> () const { safer_int<U> ret; ret.xi=xi; return ret; } // TODO check range
		// operator const char* () const { return xi; } // TODO wtf?
		template<typename U> operator U () const { U ret; ret=xi; return ret; } // TODO check range, numeric_cast?

		template<typename U> bool operator==(const safer_int<U> & obj) const { return xi==obj.xi; }
		template<typename U> bool operator!=(const safer_int<U> & obj) const { return xi!=obj.xi; }
		template<typename U> bool operator==(const U & obj) const { return xi==obj; } // TODO check!
		template<typename U> bool operator!=(const U & obj) const { return xi!=obj; } // TODO check!

		TEMPLATE bool operator==(const T_OBJECT & obj) const { return xi==obj; }
		TEMPLATE bool operator!=(const T_OBJECT & obj) const { return xi!=obj; }
		// TODO ^ enable if numeric

		template<typename U> bool operator>(safer_int<U> obj) { return xi>obj.xi; }
		template<typename U> bool operator>=(safer_int<U> obj) { return xi>=obj.xi; }
		template<typename U> bool operator<(safer_int<U> obj) { return xi<obj.xi; }
		template<typename U> bool operator<=(safer_int<U> obj) { return xi<=obj.xi; }
		template<typename U> bool operator>(U obj) { return xi>obj; }
		template<typename U> bool operator>=(U obj) { return xi>=obj; }
		template<typename U> bool operator<(U obj) { return xi<obj; }
		template<typename U> bool operator<=(U obj) { return xi<=obj; }

		safer_int<T> & operator++() { xi++; return *this; }
		safer_int<T> operator++(int) { auto tmp=*this; xi++; return tmp; }
		safer_int<T> & operator--() { xi--; return *this; }
		safer_int<T> operator--(int) { auto tmp=*this; xi--; return tmp; }

		safer_int<T> operator-() { auto tmp=*this; tmp.xi = -tmp.xi; return tmp; }

		template<typename U> safer_int<T> & operator+=(safer_int<U> obj) { xi+=obj.xi; return *this; }
		template<typename U> safer_int<T> & operator-=(safer_int<U> obj) { xi-=obj.xi; return *this; }
		template<typename U> safer_int<T> & operator/=(safer_int<U> obj) { xi/=obj.xi; return *this; }
		template<typename U> safer_int<T> & operator*=(safer_int<U> obj) { xi*=obj.xi; return *this; }
		template<typename U> safer_int<T> & operator+=(U obj) { xi+=obj; return *this; }
		template<typename U> safer_int<T> & operator-=(U obj) { xi-=obj; return *this; }
		template<typename U> safer_int<T> & operator/=(U obj) { xi/=obj; return *this; }
		template<typename U> safer_int<T> & operator*=(U obj) { xi*=obj; return *this; }

		template<typename U> safer_int<T> operator+(safer_int<U> obj) { auto tmp=*this; tmp.xi+=obj.xi; return tmp; }
		template<typename U> safer_int<T> operator-(safer_int<U> obj) { auto tmp=*this; tmp.xi-=obj.xi; return tmp; }
		template<typename U> safer_int<T> operator*(safer_int<U> obj) { auto tmp=*this; tmp.xi*=obj.xi; return tmp; }
		template<typename U> safer_int<T> operator/(safer_int<U> obj) { auto tmp=*this; tmp.xi/=obj.xi; return tmp; }
		template<typename U> safer_int<T> operator+(U obj) { auto tmp=*this; tmp.xi+=obj; return tmp; }
		template<typename U> safer_int<T> operator-(U obj) { auto tmp=*this; tmp.xi-=obj; return tmp; }
		template<typename U> safer_int<T> operator*(U obj) { auto tmp=*this; tmp.xi*=obj; return tmp; }
		template<typename U> safer_int<T> operator/(U obj) { auto tmp=*this; tmp.xi/=obj; return tmp; }
		// TODO check also for comparsion with buildint T, because it could be that build-in T is wider then some cpp_int

		void print(std::ostream& ostr) const { ostr<<xi; }
		#undef TEMPLATE
		#undef T_OBJECT
};

namespace std {

template <typename T>
class numeric_limits<safer_int<T>> {
  // based on http://www.boost.org/doc/libs/1_53_0/boost/multiprecision/cpp_int/limits.hpp
	public:
		typedef T number_type;
		typedef std::numeric_limits<T> TL;

		BOOST_STATIC_CONSTEXPR bool is_specialized = true;

		static number_type (min)() { return std::numeric_limits<number_type>::min(); };
		static number_type (max)() { return std::numeric_limits<number_type>::max(); };

		static number_type lowest() { return (min)(); }
		static number_type epsilon() { return 0; }
		static number_type round_error() { return 0; }
		static number_type infinity() { return 0; }
    static number_type quiet_NaN() { return 0; }
    static number_type signaling_NaN() { return 0; }
    static number_type denorm_min() { return 0; }

		BOOST_STATIC_CONSTEXPR int digits = TL::digits;
		BOOST_STATIC_CONSTEXPR int digits10 = TL::digits10;
		BOOST_STATIC_CONSTEXPR int max_digits10 = TL::max_digits10;
		BOOST_STATIC_CONSTEXPR bool is_signed = TL::is_signed;
		BOOST_STATIC_CONSTEXPR bool is_integer = TL::is_integer;
		BOOST_STATIC_CONSTEXPR bool is_exact = TL::is_exact;
		BOOST_STATIC_CONSTEXPR int radix = TL::radix;

	  BOOST_STATIC_CONSTEXPR int min_exponent = TL::min_exponent;
		BOOST_STATIC_CONSTEXPR int min_exponent10 = TL::min_exponent10;
		BOOST_STATIC_CONSTEXPR int max_exponent = TL::max_exponent;
		BOOST_STATIC_CONSTEXPR int max_exponent10 = TL::max_exponent10;
		BOOST_STATIC_CONSTEXPR bool has_infinity = TL::has_infinity;
		BOOST_STATIC_CONSTEXPR bool has_quiet_NaN = TL::has_quiet_NaN;
		BOOST_STATIC_CONSTEXPR bool has_signaling_NaN = TL::has_signaling_NaN;
		BOOST_STATIC_CONSTEXPR float_denorm_style has_denorm = TL::has_denorm;
		BOOST_STATIC_CONSTEXPR bool has_denorm_loss = TL::has_denorm_loss;

		BOOST_STATIC_CONSTEXPR bool is_iec559 = TL::is_iec559;
		BOOST_STATIC_CONSTEXPR bool is_bounded = TL::is_bounded;
		BOOST_STATIC_CONSTEXPR bool is_modulo = TL::is_modulo;
		BOOST_STATIC_CONSTEXPR bool traps = TL::traps;
		BOOST_STATIC_CONSTEXPR bool tinyness_before = TL::tinyness_before;
		BOOST_STATIC_CONSTEXPR float_round_style round_style = TL::round_style;

};


} // std



// Comparsions like: int > xint:

#define CONDITION typename std::enable_if< std::is_integral<U>{}>::type* = nullptr
template<typename U, typename T, CONDITION >
bool operator>(U obj, safer_int<T> safeint) { return !( (safeint < obj) || (safeint==obj) ); }

template<typename U, typename T, CONDITION >
bool operator>=(U obj, safer_int<T> safeint) { return !( (safeint < obj) || (safeint==obj) ); }

template<typename U, typename T, CONDITION >
bool operator<(U obj, safer_int<T> safeint) { return !( (safeint < obj) || (safeint==obj) ); }

template<typename U, typename T, CONDITION >
bool operator<=(U obj, safer_int<T> safeint) { return !( (safeint < obj) || (safeint==obj) ); }
// TODO XXX XXX ^--- this is error, on purpose (unit testing)
#undef CONDITION


// Comparsions like: xint > cpp_int
#define TEMPLATE \
template<typename T, \
	unsigned U1, unsigned U2, \
	boost::multiprecision::cpp_integer_type U3, boost::multiprecision::cpp_int_check_type U4, \
	typename U5, boost::multiprecision::expression_template_option U6>
#define T_OBJECT boost::multiprecision::number<boost::multiprecision::backends::cpp_int_backend<U1, U2, U3, U4, U5>, U6>

TEMPLATE bool operator>(T_OBJECT obj, safer_int<T> safeint) { return !( (safeint.xi < obj) || (safeint.xi==obj) ); }
TEMPLATE bool operator<(T_OBJECT obj, safer_int<T> safeint) { return !( (safeint.xi > obj) || (safeint.xi==obj) ); }
TEMPLATE bool operator>=(T_OBJECT obj, safer_int<T> safeint) { return !( (safeint.xi < obj) ); }
TEMPLATE bool operator<=(T_OBJECT obj, safer_int<T> safeint) { return !( (safeint.xi > obj) ); }

#undef TEMPLATE
#undef T_OBJECT

// output / streams:

template <typename T>
std::ostream& operator<<(std::ostream& ostr, safer_int<T> obj) {
	obj.print(ostr);
	return ostr;
}

typedef safer_int<basic_xint> xint;
typedef basic_uxint uxint;

template<typename T>
uxint xsize(const T & obj) {
	return uxint( obj.size() );
}

/*void foo() {
	std::cerr << is_specialization< t_correct_int , boost::multiprecision::number >::value << std::endl;
}*/

template <typename T> bool overflow_impossible_in_assign(T, T) { return true; } // same type




