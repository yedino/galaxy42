#include <apigalaxy/apigalaxy.hpp>
#include <galaxysrv_peers.hpp>

namespace apigalaxy {

bool is_valid_peer_reference(std::string simple){
	try{
		c_galaxysrv_peers::parse_peer_simplestring(simple);
	} catch(const err_check_input &){
		return false;
	}
	return true;
}

} // namespace apigalaxy
