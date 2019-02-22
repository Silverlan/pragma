#ifndef __WIICON_H__
#define __WIICON_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIIcon
	: public WITexturedRect
{
public:
	WIIcon();
	virtual ~WIIcon() override;
	virtual void Initialize() override;
	void SetClipping(uint32_t xStart,uint32_t yStart,uint32_t width,uint32_t height);
};

#endif