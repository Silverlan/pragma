// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.func_brush;
import :entities.components.render;

using namespace pragma;

void CBrushComponent::Initialize()
{
	BaseFuncBrushComponent::Initialize();
	auto pRenderComponent = static_cast<ecs::CBaseEntity &>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::World);
}
void CBrushComponent::OnEntitySpawn()
{
	BaseFuncBrushComponent::OnEntitySpawn();
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	if(m_kvSolid) {
		auto pPhysComponent = ent.GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			pPhysComponent->InitializePhysics(physics::PhysicsType::Static);
		UpdateSurfaceMaterial(ent.GetNetworkState()->GetGameState());
	}
}
void CBrushComponent::ReceiveData(NetPacket &packet)
{
	m_kvSolid = packet->Read<bool>();
	m_kvSurfaceMaterial = packet->ReadString();
}
void CBrushComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////

void CFuncBrush::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CBrushComponent>();
}
