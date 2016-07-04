
#include "project.hpp"


#include "libs0.hpp"

invalid_argument_in_version::invalid_argument_in_version( const std::string& what_arg )
: invalid_argument(std::string("This argument is not supported in this version of program; ") + what_arg)
{ }

invalid_argument_in_version::invalid_argument_in_version( const char* what_arg )
: invalid_argument(std::string("This argument is not supported in this version of program; ") + what_arg)
{ }

std::string enabled_or_disabled(bool v) {
	if (v) return "ENABLED";
	return "disabled";
}

std::string project_version_info() {
	std::ostringstream oss;
	oss << "Program build options: " << endl;
	oss << "Code level: normal code: " << enabled_or_disabled( EXTLEVEL_IS_NORMAL ) << endl;
	oss << "Code level: preview code: " << enabled_or_disabled( EXTLEVEL_IS_PREVIEW ) << endl;
	oss << "Code level: experimental code: " << enabled_or_disabled( EXTLEVEL_IS_EXPERIMENT ) << endl;
	oss << "Code level: experimental dangerous code: " << enabled_or_disabled( EXTLEVEL_IS_EXPERIMENT_DANGEROUS ) << endl;
	oss << "Enabled features: " << endl;
	oss << "  * NTRU: " << enabled_or_disabled( ENABLE_CRYPTO_NTRU ) << endl;
	oss << "  * SIDH: " << enabled_or_disabled( ENABLE_CRYPTO_SIDH ) << endl;
	return oss.str();
}

