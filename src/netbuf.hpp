#pragma once

#include "libs0.hpp"
#include <stdplus/tab.hpp>

// -------------------------------------------------------------------

class c_netchunk final {
	public:
		using t_element = unsigned char; ///< type of one elemenet

		c_netchunk(t_element * _data, size_t _size); ///< will point to memory in data (it must be valid!) will NOT free memory
		~c_netchunk()=default; ///< does nothing (does NOT delete memory!)

		c_netchunk(c_netchunk && rhs) noexcept;

		size_t size() const noexcept;
		t_element * data() const noexcept;

		stdplus::tab_view<unsigned char> to_tab_view() const;

		void shrink_to(size_t smaller_size); ///< resizes down to given size. Throw/abort if would increase size.

	public: // TODO@hb
		t_element * m_data; // points to inside of some existing t_netbuf. you do *NOT* own the data.
		size_t m_size;

		void report(std::ostream & ostr, int detail) const;
};
// -------------------------------------------------------------------


/***
	@brief Gives you a buffer of continous memory of type ::t_element (octet - unsigned char) with minimal API
*/
class c_netbuf final {
	public:
		using t_element = c_netchunk::t_element; ///< type of one elemenet

		c_netbuf(size_t size); ///< construct and allocate
		~c_netbuf(); ///< free memory
		void clear();

		c_netbuf(const c_netbuf & brother) = delete;
		c_netbuf(c_netbuf && brother) noexcept;

		c_netbuf & operator=(const c_netbuf & brother) = delete;
		c_netbuf & operator=(c_netbuf && brother) noexcept;

		size_t size() const noexcept;
		// vector<t_element> & get_data(); ///< access data
		// const vector<t_element> & get_data() const; ///< access data
		t_element & at(size_t ix); ///< access one element (asserted)

		t_element * data() noexcept;
		t_element const * data() const noexcept;

		void report(std::ostream & ostr, int detail) const;

		stdplus::tab_view<unsigned char> to_tab_view() const;
		c_netchunk get_chunk(size_t offset, size_t size); ///< returns chunk pointing to selected range from +#offset, of size #size

	private:
		t_element * m_data; ///< my actuall data storage (raw pointer because: to avoid initialization of data)
		// unique_ptr<t_element[]> m_data; ///< my actuall data storage

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
