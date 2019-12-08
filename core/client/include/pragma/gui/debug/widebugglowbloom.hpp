#ifndef __WIDEBUGGLOWBLOOM_HPP__
#define __WIDEBUGGLOWBLOOM_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugGlowBloom
	: public WITexturedRect
{
public:
	WIDebugGlowBloom();
	virtual ~WIDebugGlowBloom() override;
private:
	virtual void DoUpdate() override;
	void UpdateBloomImage();
	CallbackHandle m_cbRenderHDRMap = {};
	std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
};

#endif