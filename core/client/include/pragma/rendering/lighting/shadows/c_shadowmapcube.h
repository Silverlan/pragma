#ifndef __C_SHADOWMAPCUBE_H__
#define __C_SHADOWMAPCUBE_H__

#include "pragma/rendering/lighting/shadows/c_shadowmap.h"

class DLLCLIENT ShadowMapCube
	: public ShadowMap
{
public:
	ShadowMapCube();
	//void ClearDepth();
	//void ReloadDepthTextures(); // Vulkan TODO
	ShadowMap::Type GetType() override;
};

#endif