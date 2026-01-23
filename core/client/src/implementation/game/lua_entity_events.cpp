// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :game;
import :entities.components;

bool pragma::CGame::InvokeEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject)
{
	if(Game::InvokeEntityEvent(component, eventId, argsIdx, bInject))
		return true;
	auto *l = GetLuaState();
	if(eventId == cRenderComponent::EVENT_ON_RENDER_BOUNDS_CHANGED) {
		Lua::PushInt(l, 1);
		Lua::GetTableValue(l, argsIdx);
		auto &min = Lua::Check<Vector3>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 2);
		Lua::GetTableValue(l, argsIdx);
		auto &max = Lua::Check<Vector3>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 3);
		Lua::GetTableValue(l, argsIdx);
		auto &sphereOrigin = Lua::Check<Vector3>(l, -1);
		Lua::Pop(l, 1);

		Lua::PushInt(l, 4);
		Lua::GetTableValue(l, argsIdx);
		auto sphereRadius = Lua::CheckNumber(l, -1);
		Lua::Pop(l, 1);

		CEOnRenderBoundsChanged evData {min, max, {sphereOrigin, static_cast<float>(sphereRadius)}};
		if(bInject)
			component.InjectEvent(eventId, evData);
		else
			component.BroadcastEvent(eventId, evData);
	}
	else
		return false;
	return true;
}
