/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/networking/enums.hpp>
#include <sharedutils/netpacket.hpp>

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
