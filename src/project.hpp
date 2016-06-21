// Copyrighted (C) 2015-2016 Antinet.org team, see file LICENCE-by-Antinet.txt
#pragma once

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


