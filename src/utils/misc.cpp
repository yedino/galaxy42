
#include "utils/misc.hpp"

c_string_string_Cstyle::c_string_string_Cstyle(const vector<string> & data) {
	init_from_data(nullptr, data);
}
c_string_string_Cstyle::c_string_string_Cstyle(const string & first, const vector<string> & data) {
	init_from_data(&first, data);
}
c_string_string_Cstyle::~c_string_string_Cstyle() {
	cleanup();
}

void c_string_string_Cstyle::cleanup() {
	for (const char * ptr : argv_vec) {
		if (ptr) free( const_cast<char*>(ptr)  );
		else throw std::runtime_error( "Tried to remove a nullptr in "s + __PRETTY_FUNCTION__ );
	}
	argv_vec.empty();
}

void c_string_string_Cstyle::append(const string & s) {
	const char * newptr = strdup( s.c_str() ); // allocate...
	if (!newptr) throw std::bad_alloc(); // throw if error
	argv_vec.push_back( newptr ); // ...quickly save (to guarantee deletion)
}

void c_string_string_Cstyle::init_from_data(const string * first, const vector<string> & data) {
	try {
		argv_vec.empty();
		if (first) append(*first);
		for(const string & arg : data) append(arg);
	} catch(...) {
		cleanup();
		throw;
	}
}

const char ** c_string_string_Cstyle::get_argv() {
	const char ** ptr = argv_vec.data();
	return ptr;
}

int c_string_string_Cstyle::get_argc() const {
	return argv_vec.size();
}


