#ifndef __C_RENDERMODE_H__
#define __C_RENDERMODE_H__
#include "pragma/clientdefinitions.h"

enum class DLLCLIENT RenderMode : uint32_t
{
	None = 0,
	Auto,
	World,
	View,
	Skybox,
	Water,
	Count
};

#endif