// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef include_ui_hpp
#define include_ui_hpp

#include "libs0.hpp"

namespace ui {

/** Throw this when we just want to exit the program (though this should be called from proper top-level place in program,
 * otherwise other code can catch it with catch(...)
*/
class exception_error_exit { };


/**
 * Error that should display message to user and then exit the program, but make sure you throw this
 * from top-level function of program that is not in any catch(...) because it uses exception_error_exit
 * to quit program
 */
void action_error_exit(const string &info);

void action_info_ok(const string &info); ///< Inform user about some progress



} // namespace


#endif

