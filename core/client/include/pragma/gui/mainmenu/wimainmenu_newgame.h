/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __WIMAINMENU_NEWGAME_H__
#define __WIMAINMENU_NEWGAME_H__

#include "pragma/clientdefinitions.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"

class DLLCLIENT WIMainMenuNewGame : public WIMainMenuBase {
  protected:
	WIHandle m_hMapList;
	WIHandle m_hServerName;
	WIHandle m_hGameMode;
	WIHandle m_hRconPassword;
	WIHandle m_hMaxPlayers;
	CallbackHandle m_cbMapListReload;
	void OnStartGame(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods);
	void InitializeGameSettings();
	virtual void InitializeOptionsList(WIOptionsList *pList) override;
	using WIMainMenuBase::InitializeOptionsList;
  public:
	WIMainMenuNewGame();
	virtual ~WIMainMenuNewGame() override;
	virtual void Initialize() override;
	virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;
	void ReloadMapList();
};

#endif
