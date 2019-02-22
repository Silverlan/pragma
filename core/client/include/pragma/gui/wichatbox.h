#ifndef __WICHATBOX_H__
#define __WICHATBOX_H__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WIChatBox
	: public WIBase
{
public:
	WIChatBox();
	virtual ~WIChatBox() override;
	virtual void Initialize() override;
};

#endif