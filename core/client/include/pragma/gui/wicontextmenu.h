#ifndef __WICONTEXTMENU_H__
#define __WICONTEXTMENU_H__

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

class DLLCLIENT WIContextMenu
	: public WIRect
{
protected:
public:
	WIContextMenu();
	virtual ~WIContextMenu() override;
	virtual void Initialize() override;
	virtual void SetSize(int x,int y) override;
};

#endif