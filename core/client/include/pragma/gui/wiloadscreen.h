#ifndef __WILOADSCREEN_H__
#define __WILOADSCREEN_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class DLLCLIENT WILoadScreen
	: public WIMainMenuBase
{
private:
	WIHandle m_hText;
	WIHandle m_hProgress;
public:
	WILoadScreen();
	virtual ~WILoadScreen() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
	using WIMainMenuBase::SetSize;
};


#endif