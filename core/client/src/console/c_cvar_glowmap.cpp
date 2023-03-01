/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/debug/widebugglowbloom.hpp"

extern DLLCLIENT CGame *c_game;

void CMD_debug_glow_bloom(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_glow_bloom";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = util::to_int(argv.front());
	if(v == 0) {
		if(pEl != nullptr)
			pEl->Remove();
		return;
	}
	if(pEl != nullptr)
		return;
	pEl = wgui.Create<WIDebugGlowBloom>();
	if(pEl == nullptr)
		return;
	pEl->SetName(name);
	pEl->SetSize(256, 256);
	pEl->Update();
}
