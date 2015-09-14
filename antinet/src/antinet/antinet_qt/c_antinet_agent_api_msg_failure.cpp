#include "c_antinet_agent_api_msg_failure.hpp"

using namespace std;

c_antinet_agent_api_msg_failure::c_antinet_agent_api_msg_failure(const string &text) :
    c_antinet_agent_api_msg(text) {
    c_antinet_agent_api_msg::m_kind = e_msg_kind_failure;
}


