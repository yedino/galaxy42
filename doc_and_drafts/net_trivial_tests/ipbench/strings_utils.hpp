
#include <libs1.hpp>

struct string_as_hex {
	std::string data;

	string_as_hex()=default;
	string_as_hex(const std::string & s);
};

unsigned char hexchar2int(char c);


struct string_as_bin {
	std::string bytes;

	string_as_bin()=default;
	string_as_bin(const string_as_hex & encoded);
};



template<class T, std::size_t N>
std::string to_string( const std::array<T,N> & obj ) {
	std::ostringstream oss;
	oss<<"("<<obj.size()<<")";
	oss<<'[';
	bool first=1;
	for(auto & v : obj) { if (!first) oss << ',';  oss<<v;  first=0;  }
	oss<<']';
	return oss.str();
}

template<class T, std::size_t N>
std::string to_string_bin(  ) {

}

