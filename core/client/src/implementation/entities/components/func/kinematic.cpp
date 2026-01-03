// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.func_kinematic;
import :entities.components.render;

using namespace pragma;

void CKinematicComponent::Initialize()
{
	BaseFuncKinematicComponent::Initialize();
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::World);
}

Bool CKinematicComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvStartForward)
		StartForward();
	else if(eventId == m_netEvStartBackward)
		StartBackward();
	else
		return false;
	return true;
}

void CKinematicComponent::OnEntitySpawn()
{
	BaseFuncKinematicComponent::OnEntitySpawn();
	if(m_bInitiallyMoving == true)
		StartForward();
}

void CKinematicComponent::ReceiveData(NetPacket &packet)
{
	m_kvFirstNode = packet->ReadString();
	m_kvMoveSpeed = packet->Read<float>();
	m_bInitiallyMoving = packet->Read<bool>();
}
void CKinematicComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void CFuncKinematic::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CKinematicComponent>();
}
