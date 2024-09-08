#ifndef F_DEBUG_INFO_GLS
#define F_DEBUG_INFO_GLS

#include "../modules/sh_uniform_locations.gls"

layout(std140, LAYOUT_ID(RENDER_SETTINGS, DEBUG)) uniform Debug { uint flags; }
u_debug;

#endif
