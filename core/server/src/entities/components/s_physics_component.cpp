/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/model/s_modelmanager.h"
#include <sharedutils/scope_guard.h>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/networking/nwm_util.h>
#include <pragma/networking/enums.hpp>

using namespace pragma;

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

void SPhysicsComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BasePhysicsComponent::RegisterEvents(componentManager, registerEvent); }
void SPhysicsComponent::SendData(NetPacket &packet, networking::ClientRecipientFilter &rp)
{
	packet->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
	packet->Write<uint32_t>(static_cast<uint32_t>(GetMoveType()));
}
void SPhysicsComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void SPhysicsComponent::SetKinematic(bool b)
{
	BasePhysicsComponent::SetKinematic(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<bool>(b);
		server->SendPacket("ent_setkinematic", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SPhysicsComponent::SetMoveType(MOVETYPE movetype)
{
	BasePhysicsComponent::SetMoveType(movetype);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<unsigned char>(static_cast<unsigned char>(movetype));
	server->SendPacket("ent_movetype", p, pragma::networking::Protocol::SlowReliable);
}
void SPhysicsComponent::OnPhysicsInitialized()
{
	BasePhysicsComponent::OnPhysicsInitialized();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
		server->SendPacket("ent_phys_init", p, pragma::networking::Protocol::SlowReliable);
	}
}
void SPhysicsComponent::OnPhysicsDestroyed()
{
	BasePhysicsComponent::OnPhysicsDestroyed();
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		server->SendPacket("ent_phys_destroy", p, pragma::networking::Protocol::SlowReliable);
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
	ent.SendNetEvent(m_netEvSetCollisionsEnabled, p, pragma::networking::Protocol::SlowReliable);
}
void SPhysicsComponent::SetSimulationEnabled(bool b)
{
	if(b == GetSimulationEnabled())
		return;
	BasePhysicsComponent::SetSimulationEnabled(b);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	NetPacket p {};
	p->Write<bool>(b);
	ent.SendNetEvent(m_netEvSetSimEnabled, p, pragma::networking::Protocol::SlowReliable);
}
void SPhysicsComponent::SetCollisionType(COLLISIONTYPE collisiontype)
{
	BasePhysicsComponent::SetCollisionType(collisiontype);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p, &ent);
	p->Write<unsigned char>(static_cast<unsigned char>(collisiontype));
	server->SendPacket("ent_collisiontype", p, pragma::networking::Protocol::SlowReliable);
}

void SPhysicsComponent::SetCollisionFilter(CollisionMask filterGroup, CollisionMask filterMask)
{
	BasePhysicsComponent::SetCollisionFilter(filterGroup, filterMask);
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared()) {
		NetPacket p;
		nwm::write_entity(p, &ent);
		p->Write<unsigned int>(static_cast<unsigned int>(filterGroup));
		p->Write<unsigned int>(static_cast<unsigned int>(filterMask));
		server->SendPacket("ent_setcollisionfilter", p, pragma::networking::Protocol::SlowReliable);
	}
}

bool SPhysicsComponent::PostPhysicsSimulate()
{
	auto keepAwake = BasePhysicsComponent::PostPhysicsSimulate();
	if(GetPhysicsType() != PHYSICSTYPE::SOFTBODY)
		return keepAwake;
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	auto *phys = GetPhysicsObject();
	auto *entCl = ent.GetClientsideEntity();
	auto pPhysComponent = (entCl != nullptr) ? entCl->GetPhysicsComponent() : util::WeakHandle<BasePhysicsComponent> {};
	auto *physCl = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || physCl == nullptr)
		return;
	// Only executed on listen servers; Serverside soft-body data will be transferred to client directly
	auto &colObjs = phys->GetCollisionObjects();
	auto &colObjsCl = physCl->GetCollisionObjects();
	auto numColObjs = umath::min(colObjs.size(), colObjsCl.size());
	for(auto i = decltype(numColObjs) {0}; i < numColObjs; ++i) {
		auto &col = colObjs.at(i);
		auto &colCl = colObjsCl.at(i);
		if(col.IsValid() == false || colCl.IsValid() == false || col->IsSoftBody() == false || colCl->IsSoftBody() == false)
			continue;
		auto &sb = *col->GetSoftBody();
		auto &sbCl = *colCl->GetSoftBody();
		auto *btSb = sb.GetSoftBody();
		auto *btSbCl = sbCl.GetSoftBody();
		auto numNodes = umath::min(btSb->m_nodes.size(), btSbCl->m_nodes.size());
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
