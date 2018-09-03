// Copyrighted (C) 2015-2018 Antinet.org team, see file LICENCE-by-Antinet.txt

#pragma once
#ifndef include_ui_hpp
#define include_ui_hpp

#include "libs0.hpp"

namespace ui {

/** Throw this when we just want to exit the program (though this should be called from proper top-level place in program,
 * otherwise other code can catch it with catch(...)
*/
class exception_error_exit {
	private:
		const std::string m_what;

	public:
		exception_error_exit(const std::string &what);
		virtual ~exception_error_exit()=default;
		virtual const char* what() const;
};


/**
 * Error that should display message to user and then exit the program, but make sure you throw this
 * from top-level function of program that is not in any catch(...) because it uses exception_error_exit
 * to quit program
 */
void action_error_exit(const std::string &info);

void action_info_ok(const std::string &info); ///< Inform user about some progress


void show_error_exception(const std::string &during_action, const std::exception &e); ///< Inform user about caught exception
void show_error_exception_unknown(const std::string &during_action); ///< Inform user about caught exception of unknown type

#define UI_TRY try

// first catch is removed here, so that caller must use it in his syntax
#define _UI_CATCH_ADVANCED(DURING_ACTION, RETHROW) \
	(std::exception &e) {\
		pfp_erro("Exception caught: DURING_ACTION=["<<DURING_ACTION<<"], what="<<e.what());\
		ui::show_error_exception(DURING_ACTION, e); \
		if (RETHROW) throw; } \
	catch(...) { \
		pfp_erro("Exception caught: DURING_ACTION=["<<DURING_ACTION<<"] (unknown exception type)"); \
		ui::show_error_exception_unknown(DURING_ACTION); \
		if (RETHROW) throw; \
	}

#define UI_CATCH(DURING_ACTION) _UI_CATCH_ADVANCED(DURING_ACTION, false) do{;}while(0)
#define UI_CATCH_RETHROW(DURING_ACTION) _UI_CATCH_ADVANCED(DURING_ACTION, true) do{;}while(0)

// #define UI_CATCH_AND_EXIT(DURING_ACTION, EXIT_MSG)


} // namespace


#endif

