#include "c_agent.hpp"

namespace antinet {

#include "libs1-using.hpp" // add here using of the common namespaces and languge elements

void c_agent::execute(const t_agent_cmd &cmd) {
	if (cmd=="ping") ping();
	else unknown_command();
}

void c_agent::unknown_command() {
	cerr << "Unknown command received in agent" << endl;
}


} // namespace antinet



