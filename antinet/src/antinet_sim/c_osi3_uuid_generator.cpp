#include "c_osi3_uuid_generator.hpp"

c_osi3_uuid_generator::c_osi3_uuid_generator()
  : m_last_uuid(10000)
{
	
}

t_osi3_uuid c_osi3_uuid_generator::generate() 
{
	++m_last_uuid;
	return m_last_uuid;
}
