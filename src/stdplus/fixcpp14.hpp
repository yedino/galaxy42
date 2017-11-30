
#pragma once

namespace stdplus {

// work around for std::min / std::max problems for e.g. Debian 8 compilers even in C++14 mode

template<class T> constexpr T constexpr_max(T const & a, T const & b) { return (a>b) ? a : b; }

template<class T> constexpr T constexpr_min(T const & a, T const & b) { return (a<b) ? a : b; }

}

