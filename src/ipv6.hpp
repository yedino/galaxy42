
#pragma once

#include <stdplus/tab.hpp>

unsigned int ipv6_size_payload_from_header(stdplus::tab_view<unsigned char> tab);

unsigned int ipv6_size_entireip_from_header(stdplus::tab_view<unsigned char> tab);

