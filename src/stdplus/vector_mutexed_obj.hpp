
#pragma once

/// @TODO@rob change it to stdplus/mutex_attributes.hpp when you would move the file there
#include "../mutex.hpp"

#include "stdplus/with_mutex.hpp"

#include <vector>
#include <memory>


/// Class for reporting when no matching object was found in container,
/// e.g. in vector_mutexed_obj
class error_no_match_found : public std::exception { public:	const char * what() const noexcept override; };

/**
 * Vector of elements that are individually locked each, and that allows for safe resizing, and safe operating on elements
 * Main operations:
 * - Access and work on any random element by it's index.
 * - Search elements checking a condition (under weak lock), if matched then hard lock and work on this element.
 * - Easily grow entire vector (increase size), even if others are working on existing elements.
 * - Shrink entire vecotr (decrease size) - e.g. by locking first the elements to be erased.
 * There can be many threads running the above operations concurently.
 * @owner rfree
 */
template <typename TObj>
class vector_mutexed_obj {
	public:

		using t_one_with_mutex = stdplus::with_mutex< MutexShared, TObj >; ///< one-element (both object & it's own mutex)
		using t_one_with_mutex_ptr = std::unique_ptr< t_one_with_mutex>; ///< pointer to one-element

		vector_mutexed_obj() = default;

		vector_mutexed_obj(const vector_mutexed_obj<TObj> & other) = delete;
		vector_mutexed_obj<TObj> operator=(const vector_mutexed_obj<TObj> & other) = delete;

		void grow_to(size_t size); ///< safely increase size
		template <typename TRet, typename TFun> TRet run_on(size_t ix, TFun & fun);

//		void push_back(const TObj & obj); ///< allocate a copy of the object
		void push_back(TObj && obj); ///< allocate one-element, move given TObj rref into it
//		void push_back(t_one_with_mutex_ptr && obj_ptr); ///< move this unique-ptr one-element into us (append at end)

		/// Searches through the container until we find element for which given #fun_test function returns true,
		/// then on it we run function #fun_run - this is done #how_many times, and the result of last execution of
		/// fun_run is returned
		template <typename TRet, typename TFunTest, typename TFunRun>
		TRet run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t limit_matched=1);

		size_t size() const; ///< current size

	private:
		MutexShared m_mutex_all; ///< protects the entire vector
		std::vector< t_one_with_mutex_ptr > m_data; ///< our data
};

template <typename TObj>
void vector_mutexed_obj<TObj>::grow_to(size_t size) {
	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes),
	// but all existing elements can be worked on in background (as the ELEMENTS address does not change),
	// provided that the other operations releases their lock on m_mutex_all only when they got the raw pointer/reference
	// and loked that
	if (!(size > m_data.size())) return; // nothing to do
	m_data.resize(size);
}
/*
template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(const TObj & obj) {
//	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments
//	m_data.push_back(std::make_unique<TObj>(obj)); // allocate a copy, and push it
	// @TODO@rfree bad. look at other push_back as example
}
*/

template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(TObj && obj) {
	UniqueLockGuardRW<MutexShared> lg_all(m_mutex_all);
	// here TObj's move-constructor is used (stdplus::with_mutex forwards it)
	auto new_element = std::make_unique< t_one_with_mutex >( std::move(obj) );
	m_data.push_back( std::move(new_element) );
}

/*
template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(std::unique_ptr<TObj> && obj_ptr) {
	LockGuard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments

	auto new_obj_with_mutex = std::make_unique< stdplus::with_mutex<MutexShared, TObj > >( std::move(obj_ptr) );

	m_data.push_back( new_obj_with_mutex );
}
*/

template <typename TObj>
template <typename TRet, typename TFun>
TRet vector_mutexed_obj<TObj>::run_on(size_t ix, TFun & fun) {
	TObj * one_object_with_mutex = nullptr;
	{
		UniqueLockGuardRO<MutexShared> lg_all(m_mutex_all);
		one_object_with_mutex = m_data.at(ix).get();
		// unlocking container
	}

	{
		auto lg_one( one_object_with_mutex->get_lock_RW() );
		return fun( one_object_with_mutex->get( lg_one ) );
	}
}

/// TODO@rob
/// bool fun_test(const TObj &)
/// anything fun_run(const TObj &) ... or lambda (catured args)
/// TRet void ? - if void then "return ;"
/// TRet -> result_of....
template <typename TObj>
template <typename TRet, typename TFunTest, typename TFunRun>
TRet vector_mutexed_obj<TObj>::run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t limit_matched) {
	size_t so_far=0; // how many objects matched so far

	{
		UniqueLockGuardRO<MutexShared> lg_all(m_mutex_all);
		for (auto iter = m_data.begin(); iter<m_data.end(); ++iter) {
			t_one_with_mutex & one_object_with_mutex = * iter->get();
			const TObj * obj_ro = nullptr; // pointer instead ref - because we access it in both blocks of lock-guard/RAII
			bool matched=false;
			{ // RO look and test object
				auto lg_one_RO( one_object_with_mutex.get_lock_RO() ); // lock RO for test
				obj_ro = & one_object_with_mutex.get( lg_one_RO );
				matched = fun_test( *obj_ro ); // <--- run the test
				// unlock the one object
			}

			// no lock here, for a second - TODO

			if (matched) { // test succeeded - use the object
				auto lg_one_RW( one_object_with_mutex.get_lock_RW() ); // lock RW this time
				bool matched_again = fun_test( *obj_ro ); // <--- run the test, AGAIN, it could have change while not locked
				if (matched_again) { // really matched - we know this since we took EXCLUSIVE RW lock, so it must be true still
					TObj & obj_rw = one_object_with_mutex.get( lg_one_RW );
					++so_far;
					if (so_far >= limit_matched) {
						return fun_run( obj_rw ); // <--- run the modifier (and return, it's the last one)
					} else {
						fun_run( obj_rw ); // <--- run the modifier
					}
				} // matched_again
				// unlock the one object
			}
		} // test all objects in loop
		// unlocking container
	} // access container
	throw error_no_match_found();
}

template <typename TObj>
size_t vector_mutexed_obj<TObj>::size() const {
	return m_data.size();
}


