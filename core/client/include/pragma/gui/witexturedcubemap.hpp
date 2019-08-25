#ifndef __WITEXTUREDCUBEMAP_HPP__
#define __WITEXTUREDCUBEMAP_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_cubemapside.h"
#include <wgui/wibase.h>

namespace prosper {class Texture;};
class WITexturedRect;
class DLLCLIENT WITexturedCubemap
	: public WIBase
{
public:
	WITexturedCubemap();
	virtual void Initialize() override;
	void SetTexture(prosper::Texture &tex);
	WITexturedRect *GetSideElement(CubeMapSide side);
	void SetLOD(float lod);
protected:
	std::array<WIHandle,6> m_hCubemapSides = {};
};

#endif
