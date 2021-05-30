/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_GAME_CREATEGUIELEMENT_H__
#define __C_GAME_CREATEGUIELEMENT_H__

#include "pragma/game/c_game.h"

template<class TElement>
	TElement *CGame::CreateGUIElement(WIBase *parent)
{
	TElement *p = WGUI::GetInstance().Create<TElement>(parent);
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
