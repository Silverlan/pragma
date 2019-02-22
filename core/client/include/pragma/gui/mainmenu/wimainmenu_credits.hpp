#ifndef __WIMAINMENU_CREDITS_HPP__
#define __WIMAINMENU_CREDITS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include "pragma/gui/wichromiumpage.hpp"

class DLLCLIENT WIMainMenuCredits
	: public WIMainMenuBase,
	public WIChromiumPage
{
public:
	WIMainMenuCredits();
	virtual ~WIMainMenuCredits() override;
	virtual void Initialize() override;
protected:
	virtual void OnVisibilityChanged(bool bVisible) override;
	virtual void OnFirstEntered() override;
	virtual void InitializeWebView(WIBase *el) override;
};

#endif
