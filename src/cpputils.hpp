#pragma once

#include <cstring>
#include <cassert>

template <typename T> void memzero(const T & obj) {
	std::memset( (void*) & obj , 0 , sizeof(T) );
}

template <typename T>
class as_zerofill : public T {
	public:
		as_zerofill() {
			assert( sizeof(*this) == sizeof(T) ); // TODO move to static assert. sanity check. quote isostd
			void* baseptr = static_cast<void*>( dynamic_cast<T*>(this) );
			assert(baseptr == this); // TODO quote isostd
			memset( baseptr , 0 , sizeof(T) );
		}
		T& get() { return *this; }
};


