
#pragma once

template <typename TC> bool contains_value(const TC & c, const typename TC::value_type v) {
	return std::find( c.begin() , c.end() , v ) != c.end();
}

template <typename TC> bool contains_key(const TC & c, const typename TC::key_type k) {
	return c.find( k ) != c.end();
}

