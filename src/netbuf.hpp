#pragma once

#include "libs0.hpp"

// -------------------------------------------------------------------

class c_netchunk {
	public:
		typedef unsigned char t_element; ///< type of one elemenet

		c_netchunk(t_element * _data, size_t _size); ///< will point to memory in data (it must be valid!) will NOT free memory
		~c_netchunk()=default; ///< does nothing (does NOT delete memory!)

		size_t size() const;
		t_element * data() const;

	public:
		t_element * const m_data; // points to inside of some existing t_netbuf. you do *NOT* own the data.
		const size_t m_size;

		void report(std::ostream & ostr, int detail) const;
};
// -------------------------------------------------------------------


/***
	@brief Gives you a buffer of continous memory of type ::t_element (octet - unsigned char) with minimal API
*/
class c_netbuf final {
	public:
		typedef c_netchunk::t_element t_element; ///< type of one elemenet

		c_netbuf(size_t size); ///< construct and allocate
		~c_netbuf(); ///< free memory

		size_t size() const;
		// vector<t_element> & get_data(); ///< access data
		// const vector<t_element> & get_data() const; ///< access data
		t_element & at(size_t ix); ///< access one element (asserted)

		t_element * data();
		t_element const * data() const;

		void report(std::ostream & ostr, int detail) const;

	private:
		t_element * m_data; ///< my actuall data storage
		size_t m_size;
};
// -------------------------------------------------------------------

template <typename T>
struct c_to_report {
	public:
		const T & m_obj;
		int m_level;
		c_to_report(const T & obj, int level) : m_obj(obj), m_level(level) {}
};

template <typename TS, typename TR> TS & operator<<(TS & ostr , const c_to_report<TR> & to_report) {
	to_report.m_obj.report(ostr, to_report.m_level);
	return ostr;
}

template <typename T>
const c_to_report<T> make_report(const T & obj, int level) {
	return c_to_report<T>( obj , level);
}

// -------------------------------------------------------------------

// ============================================================================
