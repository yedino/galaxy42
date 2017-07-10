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
 * Applies a CAPs change given in #change, can do debug before/after.
 */
void security_apply_cap_change(const capmodpp::cap_statechange_full & change);

/**
 * If we are root (UID) then drops from root back to regular user who gained root by sudo, retains current CAPs
 * @warning NOT guaranteed to support double sudo (if user did e.g. sudo ./script and script does sudo ./program)
 * @pre Process must have CAPs needed for this special UID change: CAP_SETUID, CAP_SETGID, CAP_SETPCAP, CAP_CHOWN
 * @post User is not-root (not UID 0) or else exception is thrown
 */
void security_drop_root_from_sudo();

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




