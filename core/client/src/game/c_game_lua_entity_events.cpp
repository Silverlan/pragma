// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include <pragma/lua/classes/ldef_vector.h>

import pragma.client.entities.components;

bool CGame::InvokeEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject)
{
	if(Game::InvokeEntityEvent(component, eventId, argsIdx, bInject))
		return true;
	auto *l = GetLuaState();
	if(eventId == pragma::CRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &min = *Lua::CheckVector(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &max = *Lua::CheckVector(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto &sphereOrigin = *Lua::CheckVector(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		auto sphereRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		pragma::CEOnRenderBoundsChanged evData {min, max, {sphereOrigin, static_cast<float>(sphereRadius)}};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else
		return false;
	return true;
}
