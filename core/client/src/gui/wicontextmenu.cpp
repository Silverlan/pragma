#include "stdafx_client.h"
#include "pragma/gui/wicontextmenu.h"

LINK_WGUI_TO_CLASS(WIContextMenu,WIContextMenu);
WIContextMenu::WIContextMenu()
	: WIRect()
{}

WIContextMenu::~WIContextMenu()
{}

void WIContextMenu::Initialize()
{
	WIRect::Initialize();
}
void WIContextMenu::SetSize(int x,int y)
{
	WIRect::SetSize(x,y);
}