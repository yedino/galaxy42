

#include <array>
#include <sstream>
#include <exception>

namespace stdplus {


/**
 * Works like an array, but like a vector has a position so it can be added to end.
 * All elements are created once - on creation of this object, and are destroyed one - on destruction of this object.
 * push_back / pop_back does not construt / destroy objects, unlike it happens in std::vector
 * @owner
 */
template <typename T, size_t TMax>
class arrayvector {
	private:
		size_t m_current_size; ///< the current dynamic size, affected by push_back and pop_back etc.
		std::array<T,TMax> m_array; ///< our actuall data, with static max size TMax

	public:
		void assert_access(size_t ix) const;

		T & at(size_t ix);
		const T & at(size_t ix) const;
		T & operator[](size_t ix);
		const T & operator[](size_t ix) const;

		void push_back(const T & val); ///< like in vector: adds one element to end, increasing size. Throws if no more space.
		void push_back(T && val); ///< like in vector: adds one element to end, increasing size. Throws if no more space.
		void pop_back(); ///< like in vector: removes last element, BUT it does NOT destroy that element. Throws if size==0

		size_t size() const; ///< current dynamic size, affected by push_back and pop_back etc.
		constexpr size_t max_size() const; ///< the maximum possible size (it is set by TMax)
		constexpr size_t capacity() const; ///< same as max_size()
};


template <typename T, size_t TMax>
void arrayvector<T,TMax>::assert_access(size_t ix) const {
	if (ix >= m_current_size) {
		std::ostringstream oss;
		oss<<"Out of range in arrayvector with TMax="<<TMax<<", m_current_size="<<m_current_size<<", requested ix="<<ix;
		throw std::out_of_range(oss.str());
	}
}

template <typename T, size_t TMax>
T & arrayvector<T,TMax>::at(size_t ix) {
	assert_access(ix);
	return m_array[ix];
}
template <typename T, size_t TMax>
const T & arrayvector<T,TMax>::at(size_t ix) const {
	assert_access(ix);
	return m_array[ix];
}

template <typename T, size_t TMax>
T & arrayvector<T,TMax>::operator[](size_t ix) {
	assert_access(ix);
	return m_array[ix];
}
template <typename T, size_t TMax>
const T & arrayvector<T,TMax>::operator[](size_t ix) const {
	assert_access(ix);
	return m_array[ix];
}

template <typename T, size_t TMax>
size_t arrayvector<T,TMax>::size() const { return m_current_size; }

template <typename T, size_t TMax>
constexpr size_t arrayvector<T,TMax>::max_size() const { return TMax; }

template <typename T, size_t TMax>
constexpr size_t arrayvector<T,TMax>::capacity() const { return TMax; }

template <typename T, size_t TMax>
void arrayvector<T,TMax>::push_back(const T & val) {
	if (!(m_current_size<TMax)) {
		std::ostringstream oss;
		oss<<"Out of range in arrayvector with TMax="<<TMax<<", m_current_size="<<m_current_size<<", can not increase size";
	}
	m_array[m_current_size] = val;
	++m_current_size;
}
template <typename T, size_t TMax>
void arrayvector<T,TMax>::push_back(T && val) {
	if (!(m_current_size<TMax)) {
		std::ostringstream oss;
		oss<<"Out of range in arrayvector with TMax="<<TMax<<", m_current_size="<<m_current_size<<", can not increase size";
	}
	m_array[m_current_size] = std::move(val);
	++m_current_size;
}

template <typename T, size_t TMax>
void arrayvector<T,TMax>::pop_back() {
	if (!(m_current_size>0)) {
		std::ostringstream oss;
		oss<<"Out of range in arrayvector with TMax="<<TMax<<", m_current_size="<<m_current_size<<", can not decrease size";
	}
	--m_current_size;
}



} // namespace

