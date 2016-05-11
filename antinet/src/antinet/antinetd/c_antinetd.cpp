#include "c_antinetd.hpp"

namespace antinet {

#include "libs1-using.hpp" // add here using of the common namespaces and languge elements


void c_antinetd::start() { ///< starts the daemon, call it when the main program is ready
	cerr << "Antinetd: starting" << endl;
}

void c_antinetd::agent_execute(t_agent_id id, t_agent_cmd cmd) {

	auto & agents = m_agent;
	auto agent_it = agents.find(id);
	if (agent_it != agents.end()) {
		agent_it->second->execute(cmd);
	}
}


} // namespace antinet



