#ifndef __WIMAINMENU_LOADGAME_H__
#define __WIMAINMENU_LOADGAME_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class DLLCLIENT WIMainMenuLoadGame
	: public WIMainMenuBase
{
public:
	WIMainMenuLoadGame();
	virtual ~WIMainMenuLoadGame() override;
	virtual void Initialize() override;
};


#endif