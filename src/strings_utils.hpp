// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#ifndef include_strings_utils_hpp
#define include_strings_utils_hpp

#include "libs0.hpp"

#include <boost/any.hpp>

// ====================================================================


enum t_debug_style {
	e_debug_style_object=0,
	e_debug_style_short_devel=1,
	e_debug_style_crypto_devel=2,
	e_debug_style_big=2,
};

struct string_as_bin;

struct string_as_hex {
	std::string data; ///< e.g.: "1fab"

	string_as_hex()=default;
	string_as_hex(const std::string & s); ///< e.g. for "1fab" -> I will store "1fab"
	explicit string_as_hex(const string_as_bin & s); /// e.g. for "MN" I will store the hex representing bytes 0x4d(77) 0x4e(78) -> "4d4e"

	const std::string & get() const; ///< TODO(u) instead use operator<< to ostream so this is not usually needed... -- DONE?  just remove this probably
};
std::ostream& operator<<(std::ostream &ostr, const string_as_hex &obj);

bool operator==( const string_as_hex &a, const string_as_hex &b);

unsigned char int2hexchar(unsigned char i); // 15 -> 'f'
unsigned char hexchar2int(char c); // 'f' -> 15

unsigned char doublehexchar2int(string s); // "fd" -> 253


struct string_as_bin {
	std::string bytes;

	string_as_bin()=default;
	string_as_bin(const string_as_hex & encoded);
	string_as_bin(const char * ptr, size_t size); ///< build bytes from c-string data
	explicit string_as_bin(const std::string & bin); ///< create from an std::string, that we assume holds binary data

	/*template<class T>
	explicit string_as_bin( const T & obj  )
		: bytes( reinterpret_cast<const char*>( & obj ) , sizeof(obj) )
	{
		static_assert( std::is_pod<T>::value , "Can not serialize as binary data this non-POD object type");
	}*/

	template<class T, std::size_t N>
	explicit string_as_bin( const std::array<T,N> & obj )
		: bytes( reinterpret_cast<const char*>( obj.data() ) , sizeof(T) * obj.size() )
	{
		static_assert( std::is_pod<T>::value , "Can not serialize as binary data (this array type, of) this non-POD object type");
	}

	string_as_bin & operator+=( const string_as_bin & other );
	string_as_bin operator+( const string_as_bin & other ) const;
	string_as_bin & operator+=( const std::string & other );
	string_as_bin operator+( const std::string & other ) const;
	bool operator==(const string_as_bin &rhs) const;
	bool operator!=(const string_as_bin &rhs) const;
};

bool operator<( const string_as_bin &a, const string_as_bin &b);


std::string debug_simple_hash(const std::string & str);

std::string chardbg(char c); ///< Prints one character in "debugging" format, e.g. 0x0, or 0x20=32, etc.

struct string_as_dbg {
	public:
		std::string dbg; ///< this string is already nicelly formatted for debug output e.g. "(3){a,l,a,0x13}" or "(3){1,2,3}"

		string_as_dbg()=default;
		string_as_dbg(const string_as_bin & bin, t_debug_style style=e_debug_style_short_devel); ///< from our binary data string
		string_as_dbg(const char * data, size_t data_size, t_debug_style style=e_debug_style_short_devel); ///< from C style character bufer

		///! from range defined by two iterator-like objects
		template<class T>
		explicit string_as_dbg( T it_begin , T it_end, t_debug_style style=e_debug_style_short_devel )
		{
			std::ostringstream oss;
			oss << std::distance(it_begin, it_end) << ':';
			if (style==e_debug_style_crypto_devel) oss << "{hash=0x" << debug_simple_hash(std::string(it_begin, it_end)) << "}";
			oss<<'[';
			bool first=1;
			size_t size = it_end - it_begin;
			size_t size1 = 8;
			size_t size2 = 4;
			if (style==e_debug_style_big) { size1=8192; size2=128; }
			// TODO assert/review pointer operations
			if (size <= size1+size2) {
				for (auto it = it_begin ; it!=it_end ; ++it) { if (!first) oss << ','; print(oss,*it);  first=0;  }
			} else {
				{
					auto b = it_begin, e = std::min(it_end, it_begin+size1);
					for (auto it = b ; it!=e ; ++it) { if (!first) oss << ','; print(oss,*it);  first=0;  }
				}
				oss<<" ... ";
				first=1;
				{
					auto b = std::max(it_begin, it_end - size2), e = it_end;
					for (auto it = b ; it!=e ; ++it) { if (!first) oss << ','; print(oss,*it);  first=0;  }
				}
			}
			oss<<']';
			this->dbg = oss.str();
		}

		template<class T, std::size_t N> explicit string_as_dbg( const  typename std::array<T,N> & obj ) : string_as_dbg( obj.begin() , obj.end() ) { }

		operator const std::string & () const;
		const std::string & get() const;

	private:
		// print functions side affect: can modify flags of the stream os, e.g. setfill flag

		template<class T>	void print(std::ostream & os, const T & v) { os<<v; }


	public: // for chardbg.  TODO move to class & make friend class
		void print(std::ostream & os, unsigned char v, t_debug_style style=e_debug_style_short_devel );
		void print(std::ostream & os, signed char v, t_debug_style style=e_debug_style_short_devel );
		void print(std::ostream & os, char v, t_debug_style style=e_debug_style_short_devel );
};


// for debug mainly
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

// for normal use (not for debug). T must be one of: char, unsigned char, signed char (or type castable to char)
template<class T, std::size_t N>
std::string to_binary_string( const std::array<T,N> & obj ) {
	static_assert( sizeof(T) == 1 , "We should convert from array of char, or other 1-char sized T");
	return std::string( reinterpret_cast<const char*>(obj.data()), obj.size() );
}

// for normal use (not for debug)
template<class T, std::size_t N>
std::basic_string<T> to_binary_string_basic_string( const std::array<T,N> & obj ) {
	return std::basic_string<T>( obj.data(), obj.size() );
}

template<typename T>
std::ostream & debug_to_oss(std::ostream & os, const T & data, t_debug_style style) {
	if (style==e_debug_style_object) os<<data;
	else os << to_debug(data,style);
	return os;
}

std::ostream & operator<<(std::ostream & os, boost::any & obj);

template<typename TK, typename TV>
std::string to_debug(const std::map<TK,TV> & data, t_debug_style style_k=e_debug_style_object,
t_debug_style style_v=e_debug_style_object)
{
	std::ostringstream oss;
	UNUSED(data); UNUSED(style_v); UNUSED(style_k);
	for (const auto & pair : data) {

// TODO when we do bug#m153.  debug_to_oss makes no sense to connect like this - returns ostream 

//		oss << "[" << debug_to_oss(oss, pair.first, style_k) << "]";
//		oss << " -> ";
//		oss << "[" << debug_to_oss(oss, pair.second, style_v) << "]";
	}
	return oss.str();
}

template<typename TV>
std::string to_debug(const std::vector<TV> & data, t_debug_style style_v=e_debug_style_object)
{
	std::ostringstream oss;
	UNUSED(data); UNUSED(style_v);
//	for (const auto & obj : data) {
// TODO when we do bug#m153.  debug_to_oss makes no sense to connect like this - returns ostream 
//		oss << "[" << debug_to_oss(oss, obj, style_v) << "]";
//	}
	return oss.str();
}

std::string to_debug(const std::string & data, t_debug_style style=e_debug_style_short_devel);
std::string to_debug(const string_as_bin & data, t_debug_style style=e_debug_style_short_devel);
std::string to_debug(char data, t_debug_style style=e_debug_style_short_devel);
template <typename T> std::string to_debug(const T * ptr) {
	if (! ptr) return "(null)";
	std::ostringstream oss; oss << (*ptr);
	return oss.str();
}
template <typename T> std::string to_debug(const std::unique_ptr<T> & ptr) {
	if (! ptr) return "(null)";
	std::ostringstream oss; oss << (*ptr);
	return oss.str();
}

// the _b - big - wrappers/shortcuts:
std::string to_debug_b(const std::string & data);
std::string to_debug_b(const string_as_bin & data);
std::string to_debug_b(char data);
template <typename T> std::string to_debug_b(const T * ptr)
{	return to_debug_b(ptr, e_debug_style_big); }
template <typename T> std::string to_debug_b(const std::unique_ptr<T> & ptr)
{	return to_debug(ptr,e_debug_style_big); }

#endif


