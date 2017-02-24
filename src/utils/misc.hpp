
#pragma once

#include "libs0.hpp"

template <typename TC> bool contains_value(const TC & c, const typename TC::value_type v) {
	return std::find( c.begin() , c.end() , v ) != c.end();
}

template <typename TC> bool contains_key(const TC & c, const typename TC::key_type k) {
	return c.find( k ) != c.end();
}



/// Misc class to convert a vector<string> into format like argc+argv. Is RAII compatible.
class c_string_string_Cstyle final {
	public:
		c_string_string_Cstyle(const vector<string> & data);
		c_string_string_Cstyle(const string & first, const vector<string> & data);
		~c_string_string_Cstyle();
		const char ** get_argv() ;
		int get_argc() const ;
	private:
		vector< const char* > argv_vec;
		void init_from_data(const string * first, const vector<string> & data); ///< first can be null ptr; Append first (if any) and then data.
		void cleanup();
		void append(const string & s);
};

