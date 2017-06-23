// Copyrighted (C) 2017 Antinet.org team, see file LICENCE-by-Antinet.txt

/// @file Detecting/reporting/ignoring unused code, variables, dead code; Also missing return.

#pragma once

#include <tnetdbg.hpp>

/*** This function always will return true, but the point is that compiler can not prove it
 * and has to evaulate caller's code for both cases (true/false).
 * Use as: volatile_always_true<0>
*/
template<int n> bool volatile_always_true() { volatile int x=n; volatile int y=x; return (x==y); }

/// @brief This macro will silence warnings about given variable being unused
#define UNUSED(expr) do { (void)(expr); } while (0)

/** @brief This macro could be placed at end of function to guarantee that in unexpected case where that code is reached,
 * the program will abort (instead e.g. UB of returning without return expression from a non-void function).
 * Compiler can not prove that this code will always abort here, therefore this should not stop compiler
 * from issuing warnings about function possibly reaching end without return.
*/
#define DEAD_RETURN_MAYBE() do { if (volatile_always_true<0>()) { _erro("Reached unexpected end of function (DEAD_RETURN), aborting.");  std::abort(); } } while (0)

/**
 * @brief much like #DEAD_RETURN it will abort if program reaches this spot (usually at very end of function returning non-void),
 * but it says that COMPILER is MISTAKEN and fails to understand that this code is unreachable,
 * so it quiets the compiler warning about this mistake.
 * @warning Use only if you are sure that this code is unreachable, because it silences warnings; still it will not cause an UB if it was reachable after all.
 */
#define DEAD_RETURN() do { { _erro("Reached unexpected end of function (DEAD_RETURN), aborting.");  std::abort(); } } while (0)

#define TODOCODE { std::stringstream oss; oss<<"Trying to use a not implemented/TODO code, in " \
<<__func__<<" (line "<<__LINE__<<")"; \
_erro(oss.str()); \
throw std::runtime_error(oss.str()); \
} while(0)

