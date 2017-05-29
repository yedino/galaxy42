
#pragma once

/// @TODO@rob change it to stdplus/mutex_attributes.hpp when you would move the file there
#include "../mutex.hpp"

#include "stdplus/with_mutex.hpp"

#include <vector>
#include <memory>

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
		vector_mutexed_obj() = default;

		vector_mutexed_obj(const vector_mutexed_obj<TObj> & other) = delete;
		vector_mutexed_obj<TObj> operator=(const vector_mutexed_obj<TObj> & other) = delete;

		void grow_to(size_t size); ///< safely increase size
		template <typename TRet, typename TFun> TRet run_on(size_t ix, TFun & fun);

		void push_back(const TObj & obj); ///< allocate a copy of the object
		void push_back(TObj && obj); ///< allocate object, move given obj into it
		void push_back(std::unique_ptr<TObj> && obj_ptr); ///< move this unique-ptr owned object to end of this

		/// Searches through the container until we find element for which given #fun_test function returns true,
		/// then on it we run function #fun_run - this is done #how_many times, and the result of last execution of
		/// fun_run is returned
		template <typename TRet, typename TFunTest, typename TFunRun>
		TRet run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t how_many=1);

		size_t size() const; ///< current size

	private:
		MutexShared m_mutex_all; ///< protects the entire vector
		std::vector< stdplus::with_mutex< MutexShared, std::unique_ptr<TObj> > > m_data; ///< our data
};

template <typename TObj>
void vector_mutexed_obj<TObj>::grow_to(size_t size) {
	std::lock_guard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes),
	// but all existing elements can be worked on in background (as the ELEMENTS address does not change),
	// provided that the other operations releases their lock on m_mutex_all only when they got the raw pointer/reference
	// and loked that
	m_data.resize(size);
}

template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(const TObj & obj) {
	std::lock_guard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments
	m_data.push_back(std::make_unique<TObj>(obj)); // allocate a copy, and push it
	// @TODO@rfree bad. look at other push_back as example
}

template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(TObj && obj) {
	std::lock_guard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments
	m_data.push_back(std::make_unique<TObj>( std::move(obj) )); // allocate object, move data in
	// @TODO@rfree bad. look at other push_back as example
}

template <typename TObj>
void vector_mutexed_obj<TObj>::push_back(std::unique_ptr<TObj> && obj_ptr) {
	std::lock_guard<MutexShared> lg_all(m_mutex_all); // hard lock on container (avoid concurent resizes), see grow_to() comments
	m_data.push_back( stdplus::with_mutex<TObj,MutexShared>(std::move(obj_ptr)) ); // move in here entire unique_ptr
}


template <typename TObj>
template <typename TRet, typename TFun>
TRet vector_mutexed_obj<TObj>::run_on(size_t ix, TFun & fun) {
	TObj * one_object_with_mutex = nullptr;
	{
		std::unique_lock<MutexShared> lg_all(m_mutex_all, std::defer_lock);	lg_all.mutex()->lock_shared(); // read lock on container
		one_object_with_mutex = m_data.at(ix).get();
		// unlocking container
	}

	{
		auto lg_one( one_object_with_mutex->get_lock_RW() );
		return fun( one_object_with_mutex->get( lg_one ) );
	}
}

template <typename TObj>
template <typename TRet, typename TFunTest, typename TFunRun>
TRet vector_mutexed_obj<TObj>::run_on_matching(TFunTest & fun_test, TFunRun & fun_run, size_t how_many) {
	size_t so_far=0; // how many objects matched so far

	TObj * one_object_with_mutex = nullptr;
	{
		std::unique_lock<MutexShared> lg_all(m_mutex_all, std::defer_lock);	lg_all.mutex()->lock_shared(); // read lock on container
		for (auto iter = m_data.begin(); iter<m_data.end(); ++iter) {
			one_object_with_mutex = iter->get();
			TObj * obj = nullptr;
			bool matched=false;
			{
				auto lg_one_RO( one_object_with_mutex->get_lock_RO() ); // lock RO for test
				obj = one_object_with_mutex->get( lg_one_RO );
				matched = fun_test( *obj ); // <--- run the test
				// unlock the one object
			}
			if (obj != nullptr ) { // test succeeded
				auto lg_one_RW( one_object_with_mutex->get_lock_RW() ); // lock RW this time
				++so_far;
				if (so_far>=how_many) {
					return fun_run( *obj ); // <--- run the modifier (and return, it's the last one)
				} else {
					fun_run( *obj ); // <--- run the modifier
				}
				// unlock the one object
			}
		} // test all objects in loop
		// unlocking container
	} // access container
	throw std::runtime_error("Internal error! Dead code reached in vector_mutexed_obj run_on_matching."); // DEAD_RETURN();
}

template <typename TObj>
size_t vector_mutexed_obj<TObj>::size() const {
	return m_data.size();
}


