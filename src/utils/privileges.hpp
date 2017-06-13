#pragma once

/**
 * @file setting Capabilities, like the CAP capability for Linux systems.
 * @author rfree
 */

#include "platform.hpp"

namespace my_cap {

/// call this as soon as possible - to drop any privilages that are not needed anywhere
void drop_privileges_on_startup();

/// call this as soon as tuntap is configured - so that we can drop privileges for tuntap e.g. CAP_NET_ADMIN
void drop_privileges_after_tuntap();

/// call this (as soon as possible) before mainloop - so we can drop all special privileges
void drop_privileges_before_mainloop();

/// verify (or else throw) that privileges are lowered to level as low as is enough for mainloop
void verify_privileges_are_as_for_mainloop();

}




