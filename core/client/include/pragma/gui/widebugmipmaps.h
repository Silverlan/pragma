#ifndef __WIDEBUGMIPMAPS_H__
#define __WIDEBUGMIPMAPS_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

namespace prosper {class Texture;};
class DLLCLIENT WIDebugMipMaps
	: public WIBase
{
protected:
	std::vector<WIHandle> m_hTextures;
	std::vector<std::shared_ptr<prosper::Texture>> m_textures;
public:
	WIDebugMipMaps();
	virtual void Initialize() override;
	void SetTexture(const std::shared_ptr<prosper::Texture> &texture);
};

#endif
