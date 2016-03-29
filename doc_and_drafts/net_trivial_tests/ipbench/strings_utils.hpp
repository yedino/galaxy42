
#include "libs1.hpp"

struct string_as_hex {
	std::string data; ///< e.g.: "1fab"

	string_as_hex()=default;
	string_as_hex(const std::string & s);
};

unsigned char hexchar2int(char c); // 'f' -> 15

unsigned char doublehexchar2int(string s); // "fd" -> 253


struct string_as_bin {
	std::string bytes;

	string_as_bin()=default;
	string_as_bin(const string_as_hex & encoded);
	string_as_bin(const char * ptr, size_t size); ///< build bytes from c-string data
	explicit string_as_bin(std::string bin); ///< create from an std::string, that we assume holds binary data

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
};


struct string_as_dbg {
	public:
		std::string dbg; ///< this string is already nicelly formatted for debug output e.g. "(3){a,l,a,0x13}" or "(3){1,2,3}"

		string_as_dbg()=default;
		string_as_dbg(const string_as_bin & bin); ///< from our binary data string
		string_as_dbg(const char * data, size_t data_size); ///< from C style character bufer

		template<class T>
		explicit string_as_dbg( T it_begin , T it_end ) ///< from range defined by two iterator-like objects
		{
			std::ostringstream oss;
			oss<<"(" << std::distance(it_begin, it_end) << ")";
			oss<<'['; bool first=1;
			for (auto it = it_begin ; it!=it_end ; ++it) { if (!first) oss << ','; print(oss,*it);  first=0;  }
			oss<<']';
			this->dbg = oss.str();
		}

		template<class T, std::size_t N> explicit string_as_dbg( const  typename std::array<T,N> & obj ) : string_as_dbg( obj.begin() , obj.end() ) { }

		operator const std::string & () const;
		const std::string & get() const;

	private:
		// print functions side affect: can modify flags of the stream os, e.g. setfill flag

		template<class T>	void print(std::ostream & os, const T & v) { os<<v; }

		void print(std::ostream & os, unsigned char v);
		void print(std::ostream & os, signed char v);
		void print(std::ostream & os, char v);
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

