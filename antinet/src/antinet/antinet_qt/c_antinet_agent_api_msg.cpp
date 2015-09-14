#include "c_antinet_agent_api_msg.hpp"

c_antinet_agent_api_msg::c_antinet_agent_api_msg(const std::string &text) :
    m_kind(e_msg_kind_null),
    m_text(text) {
}

