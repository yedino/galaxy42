
#pragma once

#include <iostream>
#include <boost/any.hpp>

class c_transport_base_addr {
	public:
		/// the addrdata should be called from child-class and contain actually type as in t_addr in child-class
		c_transport_base_addr(boost::any addrdata);

		virtual ~c_transport_base_addr()=default;

		virtual void print(std::ostream & ostr) const ;

		/// NON-virtual function, the addr as ::any ; Caller should any_cast<> it to raw address data he wants
		const boost::any & get_addrdata() const;

	private:
		boost::any m_addrdata; ///< the address data as some (::any) datatype, see t_addr in child classes, get via get_addrdata()
};

std::ostream & operator<<(std::ostream & ostr , c_transport_base_addr & obj);

