#ifndef c_agent_idle_HPP
#define c_agent_idle_HPP

#include "libs1.hpp"
#include "c_agent.hpp"


namespace antinet {

/***
@brief This agent does basically nothing, exists e.g. for testing, checking.
*/
class c_agent_idle : public c_agent {
	virtual void ping() override;
};



} // namespace antinet


#endif // include guard

