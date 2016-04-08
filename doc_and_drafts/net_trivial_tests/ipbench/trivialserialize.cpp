

#include "trivialserialize.hpp"

#include "libs1.hpp"

using namespace std;

namespace trivialserialize {

class generator {
	protected:
		std::string m_str; ///< the generated data so far
		
	public:
		generator(size_t suggested_size);

		void push_bytes_n(size_t size, const std::string & data); ///< always same size of this field, no need to encode the length
		void push_bytes_v(size_t max_size, const std::string & data); ///< need to encode the length. And we here set the limit of it's size

		void push_byte(unsigned char c);
		void push_byte(signed char c);
		void push_byte(char c);

		void push_byte_u(unsigned char c);
		void push_byte_s(signed char c);

		const std::string & str() const; ///< get the generated string. This can be INVALIDATED by any non-const operations of this object (and ofc. after object expires)!
};

generator::generator(size_t suggested_size) 
	: m_str()
{
	m_str.reserve( suggested_size );
}

void generator::push_bytes_n(size_t size, const std::string & data) {
	assert(size == data.size()); // is the size of data the same as size that we think should go here
	m_str += data;
}

void generator::push_bytes_v(size_t max_size, const std::string & data) {
	auto size = data.size();
	assert( size < max_size );
	push_byte_u(size); // !
	m_str += data;
}

void generator::push_byte(unsigned char c) {	m_str += c; }
void generator::push_byte(signed char c) {	m_str += c; }
void generator::push_byte(char c) {	m_str += c; }

void generator::push_byte_u(unsigned char c) { 	m_str += c; }
void generator::push_byte_s(signed char c) {	m_str += c; }

const std::string & generator::str() const { return m_str; }

} // namespace


void test_trivialserialize() {
	cerr << "Tests: " << __FUNCTION__ << endl;

	trivialserialize::generator gen(50);
	gen.push_bytes_n(3, "abc");
	gen.push_bytes_v(250, "Hello");
	gen.push_byte_u(130);

	cout << "[" << gen.str() << "]" << endl;
}


