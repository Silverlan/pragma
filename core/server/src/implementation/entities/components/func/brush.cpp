// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_server.h"
#include <sharedutils/util_string.h>

module pragma.server.entities.components.func.brush;

import pragma.server.game;

using namespace pragma;

void SBrushComponent::Initialize() { BaseFuncBrushComponent::Initialize(); }
void SBrushComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(!m_kvSolid)
		return;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	UpdateSurfaceMaterial(SGame::Get());
}
void SBrushComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<bool>(m_kvSolid);
	packet->WriteString(m_kvSurfaceMaterial);
}

void SBrushComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncBrush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBrushComponent>();
}
