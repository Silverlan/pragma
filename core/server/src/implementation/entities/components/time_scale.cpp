// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"

module pragma.server.entities.components.time_scale;

import pragma.server.entities.base;

using namespace pragma;

void STimeScaleComponent::Initialize() { BaseTimeScaleComponent::Initialize(); }
void STimeScaleComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void STimeScaleComponent::SetTimeScale(float timeScale)
{
	BaseTimeScaleComponent::SetTimeScale(timeScale);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<float>(timeScale);
	ent.SendNetEvent(m_netEvSetTimeScale, p, pragma::networking::Protocol::SlowReliable);
}
