
#pragma once

#include <libs0.hpp>
#include <cable/kind.hpp>

int get_default_galaxy_port();

/**
  * Base class for representing address of given cable.
  *
  * It provides polymorphic comparsion operator op<, make it suitable to use e.g. as container keys like
  * std::map< unique_ptr< this_class > , ... > with custom comparator function that dereferences,
  * How ever instead you could just use our c_card_selector class.
  */
class c_cable_base_addr {
	protected:
		c_cable_base_addr(t_cable_kind kind); ///< creates address of given kind. Other details should be set in child class.

	public:
		c_cable_base_addr() = default; // for enable default ctor in child classes
		virtual ~c_cable_base_addr()=default;
		virtual unique_ptr<c_cable_base_addr> clone() const = 0; ///< polymorphic clone

		virtual void print(std::ostream & ostr) const = 0; ///< displays human readable form of this address

		inline bool operator==(const c_cable_base_addr &other) const;
		inline bool operator!=(const c_cable_base_addr &other) const;
		inline bool operator<(const c_cable_base_addr &other) const;

		/// Factory - from parameter like "auto:1.2.3.4", "udp:5.6.7.8:9040", and returns child class e.g. c_cable_udp_addr. Throws if invalid.
		static unique_ptr<c_cable_base_addr> cable_make_addr(const string & str);

		inline t_cable_kind get_kind() const;

	protected:
		t_cable_kind m_kind;

		/** Compare ourselves with other cable that is of same kind.
		  * @warning the caller must verify that other.kind == this->kind before calling this method, otherwise UB
		  * @return values: -1 means this < other; 0 means this == other, +1 means this > other;
		  */
		virtual signed char compare_same_class(const c_cable_base_addr & other) const = 0;

};

std::ostream & operator<<(std::ostream & ostr , const c_cable_base_addr & obj);

// ===========================================================================================================
// implementation


inline t_cable_kind c_cable_base_addr::get_kind() const { return m_kind; }

inline bool c_cable_base_addr::operator==(const c_cable_base_addr &other) const {
	if (m_kind == other.m_kind) return 0 == compare_same_class(other);
	return false;
}

inline bool c_cable_base_addr::operator!=(const c_cable_base_addr &other) const {
	if (m_kind != other.m_kind) return true;
	return 0 != compare_same_class(other);
}

inline bool c_cable_base_addr::operator<(const c_cable_base_addr &other) const {
	if (m_kind == other.m_kind) return -1 == compare_same_class(other);
	return m_kind < other.m_kind;
}


