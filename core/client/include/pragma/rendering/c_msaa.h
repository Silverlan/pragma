#ifndef __C_MSAA_H__
#define __C_MSAA_H__

#include "pragma/clientdefinitions.h"
DLLCLIENT int GetMaxMSAASampleCount();
DLLCLIENT unsigned char ClampMSAASampleCount(unsigned int *samples);

#endif