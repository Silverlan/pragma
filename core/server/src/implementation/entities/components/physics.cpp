// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.physics;

import :entities.base;
import :entities.components;
import :game;
import :model_manager;
import :server_state;

using namespace pragma;

void SPhysicsComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BasePhysicsComponent::RegisterEvents(componentManager, registerEvent); }
void SPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
	packet->Write<uint32_t>(static_cast<uint32_t>(GetMoveType()));
}
void SPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPhysicsComponent::SetKinematic(bool b)
{
	BasePhysicsComponent::SetKinematic(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<bool>(b);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETKINEMATIC, p, networking::Protocol::SlowReliable);
	}
}

void SPhysicsComponent::SetMoveType(physics::MoveType movetype)
{
	BasePhysicsComponent::SetMoveType(movetype);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<unsigned char>(static_cast<unsigned char>(movetype));
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_MOVETYPE, p, networking::Protocol::SlowReliable);
}
void SPhysicsComponent::OnPhysicsInitialized()
{
	BasePhysicsComponent::OnPhysicsInitialized();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_PHYS_INIT, p, networking::Protocol::SlowReliable);
	}
}
void SPhysicsComponent::OnPhysicsDestroyed()
{
	BasePhysicsComponent::OnPhysicsDestroyed();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_PHYS_DESTROY, p, networking::Protocol::SlowReliable);
	}
}
void SPhysicsComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BasePhysicsComponent)); }
void SPhysicsComponent::SetCollisionsEnabled(bool b)
{
	if(b == GetCollisionsEnabled())
		return;
	BasePhysicsComponent::SetCollisionsEnabled(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	NetPacket p {};
	p->Write<bool>(b);
	ent.SendNetEvent(m_netEvSetCollisionsEnabled, p, networking::Protocol::SlowReliable);
}
void SPhysicsComponent::SetSimulationEnabled(bool b)
{
	if(b == GetSimulationEnabled())
		return;
	BasePhysicsComponent::SetSimulationEnabled(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	NetPacket p {};
	p->Write<bool>(b);
	ent.SendNetEvent(m_netEvSetSimEnabled, p, networking::Protocol::SlowReliable);
}
void SPhysicsComponent::SetCollisionType(physics::CollisionType collisiontype)
{
	BasePhysicsComponent::SetCollisionType(collisiontype);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	networking::write_entity(p, &ent);
	p->Write<unsigned char>(static_cast<unsigned char>(collisiontype));
	ServerState::Get()->SendPacket(networking::net_messages::client::ENT_COLLISIONTYPE, p, networking::Protocol::SlowReliable);
}

void SPhysicsComponent::SetCollisionFilter(physics::CollisionMask filterGroup, physics::CollisionMask filterMask)
{
	BasePhysicsComponent::SetCollisionFilter(filterGroup, filterMask);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		networking::write_entity(p, &ent);
		p->Write<unsigned int>(static_cast<unsigned int>(filterGroup));
		p->Write<unsigned int>(static_cast<unsigned int>(filterMask));
		ServerState::Get()->SendPacket(networking::net_messages::client::ENT_SETCOLLISIONFILTER, p, networking::Protocol::SlowReliable);
	}
}

bool SPhysicsComponent::PostPhysicsSimulate()
{
	auto keepAwake = BasePhysicsComponent::PostPhysicsSimulate();
	if(GetPhysicsType() != physics::PhysicsType::SoftBody)
		return keepAwake;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto *phys = GetPhysicsObject();
	auto *entCl = ent.GetClientsideEntity();
	auto pPhysComponent = (entCl != nullptr) ? entCl->GetPhysicsComponent() : pragma::util::WeakHandle<BasePhysicsComponent> {};
	auto *physCl = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || physCl == nullptr)
		return;
	// Only executed on listen servers; Serverside soft-body data will be transferred to client directly
	auto &colObjs = phys->GetCollisionObjects();
	auto &colObjsCl = physCl->GetCollisionObjects();
	auto numColObjs = pragma::math::min(colObjs.size(), colObjsCl.size());
	for(auto i = decltype(numColObjs) {0}; i < numColObjs; ++i) {
		auto &col = colObjs.at(i);
		auto &colCl = colObjsCl.at(i);
		if(col.IsValid() == false || colCl.IsValid() == false || col->IsSoftBody() == false || colCl->IsSoftBody() == false)
			continue;
		auto &sb = *col->GetSoftBody();
		auto &sbCl = *colCl->GetSoftBody();
		auto *btSb = sb.GetSoftBody();
		auto *btSbCl = sbCl.GetSoftBody();
		auto numNodes = pragma::math::min(btSb->m_nodes.size(), btSbCl->m_nodes.size());
		for(auto j = decltype(numNodes) {0}; j < numNodes; ++j) {
			auto &node = btSb->m_nodes.at(j);
			auto &nodeCl = btSbCl->m_nodes.at(j);
			nodeCl.m_area = node.m_area;
			nodeCl.m_f = node.m_f;
			nodeCl.m_im = node.m_im;
			nodeCl.m_n = node.m_n;
			nodeCl.m_q = node.m_q;
			nodeCl.m_v = node.m_v;
			nodeCl.m_x = node.m_x;
		}
	}
#endif
	return keepAwake;
}
