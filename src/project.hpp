// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#pragma once

/***
 * @file Sets various project-wide settings, e.g. compilation flags, e.g. OPTION_LEVEL
*/

/***
 * @name OPTION_LEVEL_* sets do we want preview/experimental code. Use this as in #if OPTION_LEVEL_IS_PREVIEW ... #endif
 * @{
*/
#define OPTION_LEVEL_IS_EXPERIMENT_DANGEROUS OPTION_LEVEL_IS_EXPERIMENT_DANGEROUS ///< If this is true, then also enable most experimental buggy code
#define OPTION_LEVEL_IS_EXPERIMENT OPTION_LEVEL_IS_EXPERIMENT ///< If this is true, then also enable experimental buggy code
#define OPTION_LEVEL_IS_PREVIEW OPTION_LEVEL_IS_PREVIEW ///< If this is true, then also enable preview of some not-yet-stable code
#define OPTION_LEVEL_IS_NORMAL OPTION_LEVEL_IS_NORMAL ///< If this is true, then enable normal code. Should always be true probably.
/// @}

#if OPTION_LEVEL_IS_EXPERIMENT_DANGEROUS
	#if ! ( OPTION_LEVEL_IS_EXPERIMENT && OPTION_LEVEL_IS_PREVIEW && OPTION_LEVEL_IS_NORMAL )
		#error "The OPTION_LEVEL_ settings are not consistent"
	#endif
#endif

#if OPTION_LEVEL_IS_EXPERIMENT
	#if ! ( OPTION_LEVEL_IS_PREVIEW && OPTION_LEVEL_IS_NORMAL )
		#error "The OPTION_LEVEL_ settings are not consistent"
	#endif
#endif

#if OPTION_LEVEL_IS_PREVIEW
	#if ! ( OPTION_LEVEL_IS_NORMAL )
		#error "The OPTION_LEVEL_ settings are not consistent"
	#endif
#endif


