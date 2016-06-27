// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

/** @file
 *  trivialserialize.hpp -- docs for this file
 */

#ifndef include_trivialserialize_hppaaa
#define include_trivialserialize_hppaaa

#include "libs1.hpp"

// yeap defines are not so nice but also remove some problems with global constants re init ordering for example
// TODO constexpr?
#define SANE_MAX_SIZE_FOR_STRING 50000000


/**
 * @defgroup trivialserialize Trivial Serialize
 * Description: ...
 * TODO: TODO(serialize_parser_assert) - make read functions in parser that assert values and length min-max
 */
namespace trivialserialize {

namespace test {
	void test_trivialserialize();
} // namespace

// ---
/** @defgroup format_error The Format Error
 *  One description for all members of this group
 *  @{
 */

/** @page format_err_page1 This is a first section in format_error group
 *  Text of the first section
 */

/** @page format_err_page2 This is a second section in format_error group
 *  Text of the second section
 */

class format_error : public std::exception { public:	const char * what() const noexcept override; };
class format_error_read : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_badformat : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_delimiter : public format_error { public:	const char * what() const noexcept override; };


class format_error_write : public format_error { public:	const char * what() const noexcept override; };

class format_error_write_too_long : public format_error_write { public:	const char * what() const noexcept override; };

class format_error_write_value_too_big : public format_error { public:	const char * what() const noexcept override; };

class format_error_read_invalid_version : public format_error { public:	const char * what() const noexcept override; };
/** @} */	// end of format_error group


/**
 * @ingroup trivialserialize
 * @nosubgrouping
 * @brief Generates the serialized data string, from input data like bytes and strings of various types.
 * @note About string types, following are possible:
 * - conststr: strings of constant length of N characters (during read, and during write, the caller knows the size is exactyl N)
 * - octetsvarstr: We support strings that length is variable, but it is known (during read, and write, the caller tells us) to which string-length the string belongs:
 * -- is it a string with length <= 2^(8*S) -1 for S=1 (using 1 octet to encode own width)
 * -- is it a string with length <= 2^(8*S) -1 for S=2
 * -- is it a string with length <= 2^(8*S) -1 for S=3
 * -- is it a string with length <= 2^(8*S) -1 for S=4
 * -- If the program tells the string has S=3 but actuall data is always shorter then e.g. 256, so would fit in S=1, then this is fine of course, though.
 * always there is wasted space (e.g. of 1 byte) in the serialized format.
 * - varstring: we COULD have one that uses 4 bytes to encode length when it is long, but uses just 1 byte when it is e.g. 250; bitcoin uses such mechanism. Not implemented (yet?).
 * - cstring: we COULD have null-delimited string. Not implemented (yet?).
 */
class generator {
	protected:
		std::string m_str; ///< the generated data so far

		// powers of two for different number of bytes (8-bit - octets):
		constexpr static size_t bytesize0 = 1;
		constexpr static size_t bytesize1 = 1 << (8*1); // and size of 1 byte, so usable for module
		constexpr static size_t bytesize2 = 1 << (8*2); // size for 2 octet word
		constexpr static size_t bytesize3 = 1 << (8*3); // 3 byte
		constexpr static size_t bytesize4minus1 = (1LL << (8*4)) -1; // typical 4 octet bigger word, but -1 (so it fits in size_t), we need other comparsion when using it (<=).

	public:
		generator(size_t suggested_size);

		/** @name Static Interface
		 * Description: you need to specify exact (or maximum) data size youself
		 */
		///@{
		void push_byte_u(unsigned char c);
		void push_byte_s(signed char c);

		template <int S, typename T> void push_integer_u(T value); ///< Saves some unsigned integer-type S, into field that is S octets wide.
		template <int S, typename T> void push_integer_s(T value); ///< Saves some   signed integer-type S, into field that is S octets wide.

		/**
		 * @brief writes conststr (see class notes).
		 * @note Supported data size is 0 to 2^32 -1.
		 * @note Serialized size will be <= the data size.
		 * @param size - the defined exact size of data. We require (and assert) that it == data.size(). It is used ONLY for asserting.
		 * @param data - the string of binary data
		 */
		void push_bytes_n(size_t size, const std::string & data);

		/**
		 * @brief writes octetsvarstr (see class notes).
		 * @note Supported data size is 0 to 2^32 -1 in general, and for given S it is 2^(8*S)-1
		 * @note Serialized size will be <= the data size + S
		 * @param S - the numbers of octets needed to express the maximum possible size of string. String can have at most (2^(8*S)-1) characters.
		 * @param data - the string of binary data.
		 * @param max_size_for_assert - the expected maximum size of data. It can be lower, e.g. S=1 but we want to assert string is <=100 characters. Is asserted.
		 */
		template <int S> void push_bytes_sizeoctets(const std::string & data, size_t max_size_for_assert);
		/// same as push_bytes_sizeoctets with max_size_for_assert, except that argument is not used (and this additional assert is not done)
		template <int S> void push_bytes_sizeoctets(const std::string & data);
		///@}

		/** @name Dynamic Interface
		 * Description: easy handling data of any runtime size without thinking about it yourself
		 */
		///@{
		void push_integer_uvarint(uint64_t val); ///< Encode unsigned int dynamically on 1,3,5,9 octets like Bitcoin's CompactSize

		void push_varstring(const std::string &data); ///< Encode entire string of any length (but < max uint64) in dynamic format.
		///@}

		/** @name High level Interface
		 * Description: handling data structures
		 */
		///@{
		void push_vector_string(const vector<string> & data); ///< Save vector<string>

		template <typename T>
		void push_vector_object(const vector<T> & data); ///< Save vector<T>

		template <typename TKey, typename TVal>
		void push_map_object(const map<TKey,TVal> & data); ///< Save this map

		/**
		 * @brief Saves object T, also used by e.g. push_vector_object.
		 * @note If you will use this API then you must provide special function,
		 * with signature: template<> void trivialserialize::obj_serialize(const T&obj, generated &)
		*/
		template <typename T>	void push_object(const T & data);

		///@}

		/** @name Export The Result
		 * Description: export the result
		 */
		///@{
		const std::string & str() const; ///< get the generated string.
		///< This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!
		///< This GUARANTEES it will give reference to the internal-buffer, you CAN:
		///< * memlock it
		///< * you could write to it (though of course this can change the format of string and make deserialize not work)
		///< * you should NOT resize it

		std::string && str_move(); ///< gives up the stream of generated data.
		///< You should not use this object after using this function because it can be incosistent.

		///@}

		/** @name Buffer access
		 * Description: access the internal buffer for the purpose of reading it, or memlocking the memory location
		 */
		///@{
		const std::string & get_buffer() const; ///< get the generated string.
		///< This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!
		///< This GUARANTEES it will give reference to the internal-buffer, you CAN:
		///< * memlock it
		///< * you should NOT write to it
		///< * you should NOT resize it
		///@}

	protected:
		/// give number of octets of actuall-data-size, give the max_size that is just asserted, and the data
		void push_bytes_octets_and_size(unsigned char octets, size_t max_size, const std::string & data);
};


/**
 * @defgroup trivialserialize_serializefreefunctions
 * @ingroup trivialserialize
 * @brief This are the free functions that create API to serialize/deserialize given type.
 * @{
 */

// example "abstract" serialize() - it should not be used ever, instead user needs to provide own version of it
template <typename T> void obj_serialize(const T & data, trivialserialize::generator & gen) {
	UNUSED(data); UNUSED(gen);
	static_assert(templated_always_false<T>(),
		"To use this type in serialization, implement specialized template<> serialize(..) for it.");
	assert(false);
}

// example "abstract" obj_deserialize() - it should not be used ever, instead user needs to provide own version of it
class parser; // needs the forward declaration if placed here
template <typename T> T obj_deserialize(trivialserialize::parser & parser) {
	UNUSED(parser);
	static_assert(templated_always_false<T>(),
		"To use this type in deserialization, implement specialized template<> obj_deserialize(..) for it.");
	assert(false);
	T ret;
	return ret;
}

/**
 * @defgroup trivialserialize_serializefreefunctions_standardtypes
 * @ingroup trivialserialize_serializefreefunctions
 * @brief This are the free functions to (de)serialize some of the standard C++11 types
 * @{
 */
template <> void obj_serialize(const std::string & data, trivialserialize::generator & gen);
template <> std::string obj_deserialize<std::string>(trivialserialize::parser & parser);

template <> void obj_serialize(const char & data, trivialserialize::generator & gen);
template <> char obj_deserialize<char>(trivialserialize::parser & parser);

template <> void obj_serialize(const std::vector<string> & data, trivialserialize::generator & gen);
template <> std::vector<std::string> obj_deserialize<std::vector<string>>(trivialserialize::parser & parser);

/// @} //  trivialserialize_serializefreefunctions_standardtypes

/// @} //  trivialserialize_serializefreefunctions

template <typename T> void generator::push_object(const T & data) {
	obj_serialize(data, *this); // this should use the specialized user-provided function
}

template <typename T> void generator::push_vector_object(const vector<T> & data) {
	const auto size = data.size();
	assert(size <= std::numeric_limits<uint64_t>::max());
	push_integer_uvarint( data.size() );
	for (typename std::remove_cv<decltype(size)>::type i = 0; i<size; ++i) push_object(data.at(i));
}

template <typename TKey, typename TVal>
void generator::push_map_object(const map<TKey,TVal> & data) {
	push_integer_uvarint( data.size() );
	for(const auto & pair : data) {
		push_object(pair.first);
		push_object(pair.second);
	}
}

/**
 * Calculates the maximum value of unsigned integer written in binary notation that can fit given S octets.
 This function is written so that calculating of this maximum value will not even temporarly/correctly overflow
 the integers, to silence compiler warnings.
*/
template <int S> uint64_t get_max_value_of_S_octet_uint() {
	// TODO(rob): we could use unit tests for this
	static_assert( ( (S*8) <= 64) , "Size S is too big to calculate it's max value on this data types.");
	return
		( 0xFFLLU << (S-1)*8 ) // this is the FF 00 00 00 part (on example of S=4)
		| // binary-or joins both parts, since the "empty" bits in them are 0
		( (1LLU << ((S-1)*8)) - 1 ); // this is the 00 FF FF FF part, calculated from from 01 00 00 00
}

template <int S, typename T> void generator::push_integer_u(T value) {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	if (S==8) {
		if ( value >= 0xFFFFFFFFFFFFFFFF ) throw format_error_write_value_too_big();
	} else {
		if ( value >= get_max_value_of_S_octet_uint<S>() ) throw format_error_write_value_too_big();
	}

	// TODO use proper type, depending on S
	uint64_t divider = uint64_t(1) << ((S-1)*8);
	for (auto i=0; i<S; ++i) {
		//cerr << "Serializing " << value << " into S="<<S<<" octets. divider="<<divider<<"..." << flush;
		auto this_byte = (value / divider) % 256;
		//cerr << " this_byte=" << this_byte << endl;
		assert( (this_byte>=0) && (this_byte<=255) ); // TODO(r) remove assert later, when unit tests/code review
		push_byte_u( this_byte );
		divider = divider >> 8; // move to next 8 bits
	}
}

template <int S, typename T> void generator::push_integer_s(T value) {
	UNUSED(value); TODOCODE;
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data, size_t max_size_for_assert) {
	assert( data.size() <= max_size_for_assert);
	push_bytes_sizeoctets<S>(data);
}

template <int S> void generator::push_bytes_sizeoctets(const std::string & data) {
	static_assert(( (S>=1) && (S<=4) ) , "Unsupported number of octets that will express actuall-size of the data."
		"Try using 1,2,3 or 4.");
	const auto size = data.size();
	push_integer_u<S>(size);
	m_str += data; // write the actuall data
}


/**
 * @ingroup trivialserialize
 * @nosubgrouping
 * @brief For prasing serialized data. It must ncelly THROW in case if input is in format other then expected, e.g. trying to read more data then possible.
 * It absolutelly must at least assert all possible internal errors resulting from input that is logically invalid in any way (though of course
 * it must be technically valid, e.g. not nullptr string).
 * And this asserts must be always-on, because this is for parsing unsafe external data. TODO(r) abort() that can not be disabled
 */
class parser {
	private:
		// TODO(r): pick a guideline and use existing view idiom like that:
		const char * const  m_data_begin; ///< the begining of string
		const char * m_data_now; ///< current position (must be inside string, must be in range begin..end)
		const char * const m_data_end;

	public:
		struct tag_caller_must_keep_this_string_valid {} ;
		struct tag_caller_must_keep_this_buffer_valid {} ;

		parser( tag_caller_must_keep_this_string_valid x , const std::string & data_str);
		parser( tag_caller_must_keep_this_buffer_valid x , const char * buf , size_t size);

		/// === static interface - you need to specify exact (or maximum) data size youself ===

		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		unsigned char pop_byte_u();
		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		signed char pop_byte_s();
		/**
		 * \par Exception safety
		 * strong exception guarantee
		 */
		void pop_byte_skip(char c); // read expected character (e.g. a delimiter)

		// TODO(r) deduce type T fitting from S octets:
		template <int S, typename T> T pop_integer_u(); ///< Reads some unsigned integer-type S, into field that is S octets wide, as value of type T.
		template <int S, typename T> T pop_integer_s(); ///< Reads some   signed integer-type S, into field that is S octets wide, as value of type T.

		std::string pop_bytes_n(size_t size); //< Read and return binary string of exactly N characters always,
		///< that was saved using push_bytes_n(). N can be 0.
		void skip_bytes_n(size_t size); ///< as pop_bytes_n() but just skips the data
		void pop_bytes_n_into_buff(size_t size, char *buff); ///< in this version we read directly into
		///< memory buff - that must be valid block of size 'size', so [ buff .. buff+size ) must be valid memory to write into.

		template <int S> std::string pop_bytes_sizeoctets();

		/// === dynamic interface - easy handling data of any runtime size without thinking about it yourself ===

		/** @name Dynamic Interface
		 *  Description: easy handling data of any runtime size without thinking about it yourself
		 */
		///@{
		uint64_t pop_integer_uvarint(); ///< Decode unsigned int of 1,3,5,9 octets saved by push_integer_uvarint
		std::string pop_varstring(); ///< Decode string of any length saved by push_varstring()
		void skip_varstring(); ///< as pop_varstring() but skips the data
		///@}

		/** @name High Level Interface
		 * Description: handling data structures
		 */
		///@{
		vector<string> pop_vector_string(); ///< Decode a vector<string> object saved with push_vector_string

		template <typename T>
		vector<T> pop_vector_object(); ///< Decode vector<T> saved with push_vector_object<T>

		template <typename TKey, typename TVal>
		map<TKey,TVal> pop_map_object(); ///< Decode data saved with matchig push_map_object


		/**
		 * @brief Loads object T, also used by e.g. pop_vector_object.
		 * @note If you will use this API then you must provide special function,
		 * with signature: template<> void trivialserialize::obj_deserialize(const T&obj, generated &)
		*/
		template <typename T>	T pop_object(); ///< Decode object T, you must provide free function: T deserialize<T>(parser &)
		///@}

		bool is_end() const; ///< check if we are standing now at end of string (so reading anything more is not possible)

		void debug() const; ///< do some debuggging as set by devel
};

template <typename T>
T parser::pop_object() {
	T ret( obj_deserialize<T>( *this ) ); // this should use the specialized, user-provided obj_deserialize() free function.
	return ret;
}

template <typename T>
vector<T> parser::pop_vector_object() {
	vector<T> ret;
	auto size = pop_integer_uvarint(); // TODO const
	// assert( size <= (1LLU << 64LLU) ); // TODO
	for (decltype(size) i = 0; i<size; ++i) ret.push_back( pop_object<T>() );
	return ret;
}

template <typename TKey, typename TVal>
map<TKey,TVal> parser::pop_map_object() {
	bool dbg=0;
	if (dbg) _dbg1("Reading map");

	map<TKey,TVal> ret;
	auto size = pop_integer_uvarint(); // TODO const
	if (dbg) _dbg1("Reading map size="<<size);
	// assert( size <= (1LLU << 64LLU) ); // TODO
	for (decltype(size) i = 0; i<size; ++i) {
		if (dbg) _dbg1("Reading i=" << i);
		TKey key = pop_object<TKey>();
		TVal value = pop_object<TVal>();

		auto size_old = ret.size();
		auto found = ret.find(key);
		if (found != ret.end()) throw format_error_read_badformat(); // there was already such key
		if (dbg) _dbg1("Inserting");
		ret.emplace(std::move(key), std::move(value));
		if (ret.size() != size_old+1) throw format_error_read_badformat(); // the insert failed apparently
	}
	if (ret.size() != size) throw format_error_read_badformat(); // the resulting size somehow was different then expected
	return ret;
}


template <int S, typename T> T parser::pop_integer_u() {
	static_assert( std::is_unsigned<T>() , "This function saves only unsigned types, pass it unsigned or use other function.");
	static_assert( S>0 , "S must be > 0");
	static_assert( S<=8 , "S must be <= 8");

	// TODO test if type T is not overflowing

	T value=0;
	uint64_t multiplier = uint64_t(1) << ((S-1)*8);

	for (auto i=0; i<S; ++i) {
		unsigned int this_byte = pop_byte_u();
		// cerr << "Reading for S="<<S<<" this_byte = " << this_byte << " mul=" << multiplier << endl;
		value += this_byte * multiplier;
		multiplier = multiplier >> 8; // move to next 8 bits
	}
	return value;
}

template <int S, typename T> T parser::pop_integer_s() {
	throw std::runtime_error("Not implemented yet");
}


template <int S> std::string parser::pop_bytes_sizeoctets() {
	size_t size = pop_integer_u<S,size_t>();
	// here we have the size, and we moved pass the serialized bytes of size, string is next:
	return pop_bytes_n( size );
}


} // namespace


#endif

