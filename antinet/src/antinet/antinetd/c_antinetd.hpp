#ifndef c_antinetd_HPP
#define c_antinetd_HPP

#include "libs1.hpp"

#include "c_agent.hpp"



namespace antinet {

/***
@brief Implementation of the highest level of antinetd (antinet daemon) - to be used from main() of antinetd, or of programs embedding this server
*/
class c_antinetd { 
	public:
		void start(); ///< starts the daemon, call it when the main program is ready

		void agent_execute(t_agent_id id, t_agent_cmd cmd);

	private:
		std::map< t_agent_id , std::unique_ptr< c_agent > > m_agent;
};



} // namespace antinet


#endif // include guard

