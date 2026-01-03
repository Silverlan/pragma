// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.env_timescale;
using namespace pragma;

void CEnvTimescaleComponent::ReceiveData(NetPacket &packet)
{
	m_kvTimescale = packet->Read<float>();
	m_kvInnerRadius = packet->Read<float>();
	m_kvOuterRadius = packet->Read<float>();
}
void CEnvTimescaleComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

//////////

void CEnvTimescale::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CEnvTimescaleComponent>();
}
