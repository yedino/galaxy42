#include "c_antinet_agent_api_msg.h"

c_antinet_agent_api_msg::c_antinet_agent_api_msg(const std::string &text) :
    m_text(text),
    m_kind(e_msg_kind_null)
{
}
