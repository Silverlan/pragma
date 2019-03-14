#ifndef __WIDEBUGSSAO_HPP__
#define __WIDEBUGSSAO_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIDebugSSAO
	: public WITexturedRect
{
public:
	WIDebugSSAO();
	virtual void Update() override;
	void SetUseBlurredSSAOImage(bool b);
private:
	bool m_bUseBlurVariant = false;
};

#endif