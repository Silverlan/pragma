// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.func.brush;

import :game;

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
		pPhysComponent->InitializePhysics(physics::PhysicsType::Static);
	UpdateSurfaceMaterial(SGame::Get());
}
void SBrushComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<bool>(m_kvSolid);
	packet->WriteString(m_kvSurfaceMaterial);
}

void SBrushComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncBrush::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBrushComponent>();
}
