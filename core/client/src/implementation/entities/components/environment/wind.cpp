// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.env_wind;
using namespace pragma;

void CWindComponent::ReceiveData(NetPacket &packet)
{
	auto windForce = packet->Read<Vector3>();
	SetWindForce(windForce);
}
void CWindComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CEnvWind::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CWindComponent>();
}
