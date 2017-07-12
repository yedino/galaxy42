#pragma once

/**
 * @file setting Capabilities, like the CAP capability for Linux systems.
 * @author rfree
 * @warning TODO support of regular_user->root_user by means of starting program that is SUID-root,
 * ...now we probably incorrect detect regular user id (from env sudo)
 * @warning TODO support of root->regular_user by means of starting program that is SUID-regular user
 * ...program should ignore that it has getuid()==0 if geteuid()==0 - easy but first need to verify
 * ...that this is the correct safe solution
 */

#include "platform.hpp"
#include <utils/capmodpp.hpp>

namespace my_cap {

/**
 * Returns summary of allowed CAPs, also UID/GID, and possibly other details
 * In case of errors with checking state it should usually return proper information in the string, not throw.
 * @param verbose if yes then produce one-liner short info, else more detailed text (with \n endlines)
 */
std::string get_security_info(bool verbose = false) noexcept;

// ===========================================================================================================
// commands for this project:

void drop_root();

/// call this as soon as possible - to drop any privilages that are not needed anywhere
void drop_privileges_on_startup();

/// call this as soon as tuntap is configured - so that we can drop privileges for tuntap e.g. CAP_NET_ADMIN
void drop_privileges_after_tuntap();

/// call this (as soon as possible) before mainloop - so we can drop all special privileges
void drop_privileges_before_mainloop();

/// verify (or else throw) that privileges are lowered to level as low as is enough for mainloop
void verify_privileges_are_as_for_mainloop();

}




