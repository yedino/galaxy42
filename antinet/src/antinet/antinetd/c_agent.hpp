#ifndef c_agent_HPP
#define c_agent_HPP

#include "libs1.hpp"

typedef std::string t_agent_id;

typedef std::string t_agent_cmd;

namespace antinet {

/***
@brief Instance of agent: a virtual program that realizes some function for the Antinet server (e.g. agent to find connections, other one to run exchange, other to mint tokens)
*/
class c_agent {

	public:
		virtual void ping()=0;

		virtual void execute(const t_agent_cmd &cmd);

	private:
		virtual void unknown_command();

};



} // namespace antinet


#endif // include guard

