#ifndef __WISNAPAREA_HPP__
#define __WISNAPAREA_HPP__

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>

class DLLCLIENT WISnapArea
	: public WIBase
{
public:
	WISnapArea();
	virtual void Initialize() override;
	WIBase *GetTriggerArea();
private:
	WIHandle m_hTriggerArea = {};
};

#endif