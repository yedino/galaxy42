
#pragma once

#include <iostream>

class c_transport_base_addr {
	public:
		virtual ~c_transport_base_addr()=default;
		virtual void print(std::ostream & ostr) const ;
};

std::ostream & operator<<(std::ostream & ostr , c_transport_base_addr & obj);

