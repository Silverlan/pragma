#ifndef __C_GAME_CREATEGUIELEMENT_H__
#define __C_GAME_CREATEGUIELEMENT_H__

#include "pragma/game/c_game.h"

template<class TElement>
	TElement *CGame::CreateGUIElement(WIBase *parent)
{
	TElement *p = WGUI::Create<TElement>(parent);
	if(p == nullptr)
		return nullptr;
	//InitializeGUIElement(p);
	return p;
}
template<class TElement>
	TElement *CGame::CreateGUIElement(WIHandle *hParent)
{
	WIBase *pParent = nullptr;
	if(hParent != nullptr && hParent->IsValid())
		pParent = hParent->get();
	return CreateGUIElement<TElement>(pParent);
}

#endif
