// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#pragma once

#include <stdexcept>
#include <string>

class invalid_argument_in_version : public std::invalid_argument {
	public:
		explicit invalid_argument_in_version( const std::string& what_arg );
		explicit invalid_argument_in_version( const char* what_arg );
};


/***
 * @file Sets various project-wide settings, e.g. compilation flags, e.g. EXTLEVEL
*/

/***
 * @name EXTLEVEL_* sets do we want preview/experimental code. Use this as in #if EXTLEVEL_IS_PREVIEW ... #endif
 * @{
*/
#define EXTLEVEL_IS_EXPERIMENT_DANGEROUS EXTLEVEL_IS_EXPERIMENT_DANGEROUS_CMAKE ///< If this is true, then also enable most experimental buggy code
#define EXTLEVEL_IS_EXPERIMENT EXTLEVEL_IS_EXPERIMENT_CMAKE ///< If this is true, then also enable experimental buggy code
#define EXTLEVEL_IS_PREVIEW EXTLEVEL_IS_PREVIEW_CMAKE ///< If this is true, then also enable preview of some not-yet-stable code
#define EXTLEVEL_IS_NORMAL EXTLEVEL_IS_NORMAL_CMAKE ///< If this is true, then enable normal code. Should always be true probably.
/// @}

#if EXTLEVEL_IS_EXPERIMENT_DANGEROUS
	#if ! ( EXTLEVEL_IS_EXPERIMENT && EXTLEVEL_IS_PREVIEW && EXTLEVEL_IS_NORMAL )
		#error "The EXTLEVEL_ settings are not consistent"
	#endif
#endif

#if EXTLEVEL_IS_EXPERIMENT
	#if ! ( EXTLEVEL_IS_PREVIEW && EXTLEVEL_IS_NORMAL )
		#error "The EXTLEVEL_ settings are not consistent"
	#endif
#endif

#if EXTLEVEL_IS_PREVIEW
	#if ! ( EXTLEVEL_IS_NORMAL )
		#error "The EXTLEVEL_ settings are not consistent"
	#endif
#endif


/***
 * @name ENABLE_* flags - copy them from the ones set by CMake. This also allows us to add doxygen for them
 * @{
*/
#define ENABLE_CRYPTO_SIDH ENABLE_CRYPTO_SIDH_CMAKE ///< should the SIDH crypto library be used
#define ENABLE_CRYPTO_NTRU ENABLE_CRYPTO_NTRU_CMAKE ///< should the NTru crypto library be used
/// @}

#if ENABLE_CRYPTO_SIDH
	#include <SIDH.h>
#endif

#if ENABLE_CRYPTO_NTRU
	#include "ntru/include/ntru_crypto_drbg.h"
#endif

#ifndef USE_BOOST_MULTIPRECISION
	#error "Not defined USE_BOOST_MULTIPRECISION"
#endif


std::string project_version_info();

