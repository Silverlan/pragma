// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.time_scale;

import :entities.base;

using namespace pragma;

void STimeScaleComponent::Initialize() { BaseTimeScaleComponent::Initialize(); }
void STimeScaleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void STimeScaleComponent::SetTimeScale(float timeScale)
{
	BaseTimeScaleComponent::SetTimeScale(timeScale);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == false)
		return;
	NetPacket p;
	p->Write<float>(timeScale);
	ent.SendNetEvent(m_netEvSetTimeScale, p, networking::Protocol::SlowReliable);
}
