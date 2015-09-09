#ifndef INCLUDE_C_use_opengl_hpp
#define INCLUDE_C_use_opengl_hpp


#include "libs1.hpp"

/**
This loads the OpenGL libraries, includes etc - if OpenGL support is enabled.
Else should do nothing (or create some stubs).

Safe to use (to include) even when opengl support is disabled.
*/


/*

API:
This file will:

--- macro ---
sets macro USE_OPENGL to 0 or 1

--- include ---

will include the basic files needed to have access to OpenGL / basic OGL-tookit header

*/

#ifndef USE_OPENGL
	#warning "USE_OPENGL - this option (C macro) was not defined. It should bedefined by the make/cmake system usually"
	#define USE_OPENGL 1
#else
	#if USE_OPENGL
		#define USE_OPENGL 1
	#else
		#define USE_OPENGL 0
	#endif
#endif



#if USE_OPENGL

	#include <alleggl.h>

#endif






#endif // include guard

