
#pragma once

#include "libs0.hpp"

template <typename TC> bool contains_value(const TC & c, const typename TC::value_type & v) {
	return std::find( c.begin() , c.end() , v ) != c.end();
}

template <typename TC> bool contains_key(const TC & c, const typename TC::key_type & k) {
	return c.find( k ) != c.end();
}

template <typename TC> void remove_all(TC & c, const typename TC::value_type & v) {
	c.erase( remove(c.begin() , c.end() , v) , c.end());
}

template <typename TC> size_t remove_and_count(TC & c, const typename TC::value_type & v) {
	size_t size_before = c.size();
	c.erase( remove(c.begin() , c.end() , v) , c.end());
	size_t size_after = c.size();
	_check_abort( size_after <= size_before );
	size_t removed =  size_before - size_after;
	return removed;
}

/// Misc class to convert a vector<string> into format like argc+argv. Is RAII compatible.
class c_string_string_Cstyle final {
	public:
		c_string_string_Cstyle(const std::vector<std::string> & data);
		c_string_string_Cstyle(const std::string & first, const std::vector<std::string> & data);
		~c_string_string_Cstyle();
		const char ** get_argv() ;
		int get_argc() const ;
	private:
		std::vector< const char* > argv_vec;
		void init_from_data(const std::string * first, const std::vector<std::string> & data); ///< first can be null ptr; Append first (if any) and then data.
		void cleanup();
		void append(const std::string & s);
};

class init_ptr_checker {
	public:
		template<typename T, typename ...Types>
		init_ptr_checker(T ptr, const Types&... ptrs) : init_ptr_checker(ptrs...) {
			static_assert(std::is_pointer<T>::value, "Type is not pointer");
			if (ptr == nullptr) throw std::invalid_argument("nullptr used");
		}

		template<typename T, typename ...Types>
		init_ptr_checker(std::shared_ptr<T> &ptr, const Types&... ptrs) : init_ptr_checker(ptrs...) {
			if (ptr == nullptr) throw std::invalid_argument("nullptr used");
		}

		template<typename T, typename ...Types>
		init_ptr_checker(std::unique_ptr<T> &ptr, const Types&... ptrs) : init_ptr_checker(ptrs...) {
			if (ptr == nullptr) throw std::invalid_argument("nullptr used");
		}

		init_ptr_checker() = default;
		virtual ~init_ptr_checker() = default;
};
