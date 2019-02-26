#ifndef __VR_INCLUDES_H__
#define __VR_INCLUDES_H__

#ifdef USE_OPENGL_OFFSCREEN_CONTEXT
#ifndef __glew_h__
	#ifndef GLEW_STATIC
		#define GLEW_STATIC
	#endif
	#include <GL/glew.h>
#endif
#endif

#endif
