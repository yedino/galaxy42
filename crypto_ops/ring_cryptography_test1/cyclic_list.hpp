#ifndef RING_CRYPTOGRAPHY_CYCLIC_LIST_H
#define RING_CRYPTOGRAPHY_CYCLIC_LIST_H

#include <vector>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <stdexcept>

using std::vector;
using std::initializer_list;
using std::cout;

template <class T>
class cyclic_list {
private:
		vector<T> inner_list;
		typename vector<T>::iterator current_it;

public:
		cyclic_list () : current_it(inner_list.end()) { }

		cyclic_list (initializer_list<T> init) : inner_list(init), current_it(inner_list.end()) { }

		cyclic_list (const cyclic_list &rhs) : inner_list(rhs.inner_list), current_it(rhs.current_it) { }

		cyclic_list (cyclic_list &&rhs) : cyclic_list() { swap(*this, rhs); }

		~cyclic_list () = default;

		void next () {
			if (size() == 0)
				throw std::runtime_error("empty list");

			++current_it;
			if (current_it == inner_list.end())
				current_it = inner_list.begin();
		}

		void prev () {
			if (size() == 0)
				throw std::runtime_error("empty list");

			if (current_it == inner_list.begin())
				current_it = std::prev(inner_list.end());
			else
				--current_it;
		}

		T &get_current_element () { return *current_it; }

		typename vector<T>::iterator begin () { return inner_list.begin(); }

		typename vector<T>::iterator end () { return inner_list.end(); }

		size_t size () const { return inner_list.size(); }

		void push (const T &value) {
			inner_list.push_back(value);
			if (size() == 1)
				current_it = inner_list.begin();
		}

		void push (T &&value) {
			inner_list.push_back(value);
			if (size() == 1)
				current_it = inner_list.begin();
		}

		void push (initializer_list<T> init) { // TODO
			throw std::runtime_error("not implemented function push(initializer_list) !");
//			for (auto it = init.begin(); it != init.end(); ++it)
//				inner_list.push_back(*it);
//			for (size_t i = 0; i < init.size(); ++i)
//				inner_list.push_back(init[i]);

//			inner_list.insert(init.begin(), init.end());
		}

		void erase_current_element () {
			if (current_it == inner_list.end())
				throw std::runtime_error("current element is list<T>::end");

			if (size() == 0)

				throw std::runtime_error("empty list");

			auto backup = current_it;
			next();
			inner_list.erase(backup);
		}

		void print (std::ostream &stream = cout) {
			auto backup = current_it;
			do {
				stream << get_current_element();
				next();
			} while (current_it != backup);
		}

		friend void swap (cyclic_list &a, cyclic_list &b) {
			std::swap(a.inner_list, b.inner_list);
			std::swap(a.current_it, b.current_it);
		}

		void for_each (void func (T &)) {
			auto backup = current_it;
			do {
				func(get_current_element());
				next();
			} while (current_it != backup);
		}

		cyclic_list &operator= (cyclic_list rhs) {
			swap(*this, rhs);
			return *this;
		}
};

#endif //RING_CRYPTOGRAPHY_CYCLIC_LIST_H

// TODO remove print()
// TODO implement const for_each
// TODO add const specifier
// TODO maybe more methods
// TODO implement access to random element (for start_shuffle usage)
