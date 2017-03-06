
#pragma once

#include "libs1.hpp"

int get_default_galaxy_port();

class c_cable_base_addr {
	protected:
		/// the addrdata should be called from child-class and contain actually type as in t_addr in child-class
		c_cable_base_addr(boost::any && addrdata);

		c_cable_base_addr()=default; ///< empty constr. You should next call init_addrdata() fro child-class

	public:
		virtual ~c_cable_base_addr()=default;

		/// replace current addr-data with given one, use this from child-class'es constr
		void init_addrdata(boost::any && addrdata);

		/// NON-virtual function, the addr as ::any ; Caller should any_cast<c_..._addr::t_addr> it to raw address data he wants
		const boost::any & get_addrdata() const;

		virtual void print(std::ostream & ostr) const ; ///< displays human readalbe form of this address

		virtual std::string cable_type_name() const =0; ///< return name of type of this cable, e.g.: "tcp" "udp" "ETH" "shm"

		/// is my address the same (cable type, and address) to another.
		virtual bool is_same(const c_cable_base_addr &other) const =0;

		/// return -1 if I am smaller, 0 if same, +1 if bigger, then the other address. Compares also across different cable-types
		virtual int compare(const c_cable_base_addr &other) const =0;

		/// Factory - from parameter like "auto:1.2.3.4", "udp:5.6.7.8:9040", and returns child class e.g. c_cable_udp_addr. Throws if invalid.
		static unique_ptr<c_cable_base_addr> cable_make_addr(const string & str);

	private:
		boost::any m_addrdata; ///< the address data as some (::any) datatype, see t_addr in child classes, get via get_addrdata()
};

std::ostream & operator<<(std::ostream & ostr , c_cable_base_addr & obj);

bool operator==(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2);
bool operator<(const c_cable_base_addr & obj1, const c_cable_base_addr & obj2);

