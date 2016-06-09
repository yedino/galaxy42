#pragma once
#ifndef LIBS1_HPP
#define LIBS1_HPP

#include <memory>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <iostream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <cmath>
#include <random>
#include <algorithm>
#include <exception>
#include <mutex>
#include <cassert>
#include <array>
#include <set>

//for libpng
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include <png.h>
#include <allegro.h>
#include "loadpng.hpp"

#include "c_tnetdbg.hpp"

using std::string;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;

using std::make_shared;

using std::vector;
using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::unordered_set;

using std::ostream;
using std::ifstream;
using std::istringstream;

using std::endl;

#include "c_tnetdbg.hpp"

/***
 * type that is a size_t, but can be with error-signaling value e.g. -1, otherwise it is valid.
 * the caller MUST check the returned value against size_t_is() before using it
 */
typedef size_t size_t_maybe; 

inline size_t_maybe size_t_invalid() { ///< returns a size_t that means "invalid size_t"
	return static_cast<size_t>( -1 );
}

/***
 * returns if given size_t is correct, or is it the invalid value
*/
inline bool size_t_is_ok(size_t x) {
	if (x == size_t_invalid()) return false;
	return true;
}

// extending the std with helpfull tools
namespace std {


// this is due to enter C++14
// http://stackoverflow.com/questions/7038357/make-unique-and-perfect-forwarding
template <typename T, typename... Args>
std::unique_ptr<T> make_unique (Args &&... args) {
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

using std::make_unique;


// extending the std with helpfull tools - by own idea
namespace stdplus {

template <typename T, typename U>
T& unique_cast_ref(std::unique_ptr<U> & u) {
	return dynamic_cast<T&>( * u.get() );
}

template <typename T, typename U>
T* unique_cast_ptr(std::unique_ptr<U> & u) {
	return dynamic_cast<T*>( u.get() );
}

// idea from http://www.g-truc.net/post-0708.html
// in C++17 this will be not needed - https://isocpp.org/files/papers/n4280.pdf
template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept {
	return N;
}
template <class T>
std::size_t countof(T const & obj) {
	return obj.size();
}


}

using namespace stdplus;


#endif

