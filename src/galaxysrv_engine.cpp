
#include "galaxysrv_engine.hpp"


c_weld::c_weld(size_t memsize)
:
m_empty(true),
// Init here things that are documented in m_empty as always valid:
m_buf( memsize )
{
}

