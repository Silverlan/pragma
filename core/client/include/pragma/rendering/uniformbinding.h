#ifndef __UNIFORMBINDING_H__
#define __UNIFORMBINDING_H__

#include "pragma/clientdefinitions.h"
#include <inttypes.h>

enum class DLLCLIENT UniformBinding : uint32_t
{
	ViewProjection = 1,
	Time = ViewProjection +1,
	AnimatedTexture = Time +1,
	Fog = AnimatedTexture +1,
	CSM = Fog +1,
	LightIds = CSM +1,
	Debug = LightIds +1
};

#endif
