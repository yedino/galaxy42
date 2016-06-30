// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#pragma once

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


