#ifndef __OPENGL_HELPER_H__
#define __OPENGL_HELPER_H__

#include "pragma/clientdefinitions.h"

DLLCLIENT void generate_frame_buffer(unsigned int *frameBuffer,unsigned int *textures,unsigned int samples=0,unsigned int *depthTexture=nullptr,unsigned int numColorAttachments=1);

#endif
