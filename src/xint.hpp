#pragma once

#include <iostream>
#include <ostream>
#include <string>

#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
basic_xint;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<64, 64,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void> >
basic_uxint;


typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<1024, 1024,
		boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void> >
xbigint;

typedef boost::multiprecision::number<
	boost::multiprecision::cpp_int_backend<1024, 1024,
		boost::multiprecision::unsigned_magnitude, boost::multiprecision::checked, void> >
uxbigint;

template<typename T>
class safer_int {
	private:
		T xi; // the xint implementig my basic type
	public:
		safer_int()
			: xi( T() )
		{
		}

		template<typename U> safer_int(const safer_int<U> & obj)
			: xi(obj.xi)
		{
			// TODO numeric_cast
		}
		safer_int(int64_t obj) // TODO
			: xi(obj)
		{
			// TODO numeric_cast
		}
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
		operator const char* () const { return ""; } // return xi.operator const char*(); } // TODO wtf?
		template<typename U> operator U () const { U ret; ret=xi; return ret; } // TODO check range, numeric_cast?

		template<typename U> bool operator==(const U & obj) const { return xi==obj; }
		template<typename U> bool operator!=(const U & obj) const { return xi!=obj; }

		template<typename U> bool operator>(safer_int<U> obj) { return xi>obj.xi; }
		template<typename U> bool operator>=(safer_int<U> obj) { return xi>=obj.xi; }
		template<typename U> bool operator<(safer_int<U> obj) { return xi<obj.xi; }
		template<typename U> bool operator<=(safer_int<U> obj) { return xi<=obj.xi; }

		safer_int<T> & operator++() { xi++; return *this; }
		safer_int<T> operator++(int) { auto tmp=*this; xi++; return tmp; }
		safer_int<T> & operator--() { xi--; return *this; }
		safer_int<T> operator--(int) { auto tmp=*this; xi--; return tmp; }

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

		void print(std::ostream& ostr) const { ostr<<xi; }
};

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




