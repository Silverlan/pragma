#include "stdafx_server.h"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/model/s_modelmanager.h"
#include "pragma/networking/wvserverclient.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <sharedutils/scope_guard.h>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/damageable_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/networking/nwm_util.h>

using namespace pragma;

extern DLLSERVER ServerState *server;
extern DLLSERVER SGame *s_game;

void SPhysicsComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	BasePhysicsComponent::RegisterEvents(componentManager);
}
void SPhysicsComponent::SendData(NetPacket &packet,nwm::RecipientFilter &rp)
{
	packet->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
	packet->Write<uint32_t>(static_cast<uint32_t>(GetMoveType()));
}
luabind::object SPhysicsComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPhysicsComponentHandleWrapper>(l);}

void SPhysicsComponent::InitializeBrushGeometry()
{
	BasePhysicsComponent::InitializeBrushGeometry();
	auto hModel = ModelManager::CreateFromBrushMeshes(s_game,m_brushMeshes,s_game->GetSurfaceMaterials());
	if(GetEntity().IsWorld() == true && hModel != nullptr)
		hModel->Optimize();
	hModel->Update(ModelUpdateFlags::All);
	auto mdlComponent = GetEntity().AddComponent<SModelComponent>();
	if(mdlComponent.valid())
		mdlComponent->SetModel(hModel);
}

void SPhysicsComponent::SetKinematic(bool b)
{
	BasePhysicsComponent::SetKinematic(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<bool>(b);
		server->BroadcastTCP("ent_setkinematic",p);
	}
}

void SPhysicsComponent::SetMoveType(MOVETYPE movetype)
{
	BasePhysicsComponent::SetMoveType(movetype);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<unsigned char>(static_cast<unsigned char>(movetype));
	server->BroadcastTCP("ent_movetype",p);
}
void SPhysicsComponent::OnPhysicsInitialized()
{
	BasePhysicsComponent::OnPhysicsInitialized();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<unsigned int>(static_cast<unsigned int>(m_physicsType));
		server->BroadcastTCP("ent_phys_init",p);
	}
}
void SPhysicsComponent::OnPhysicsDestroyed()
{
	BasePhysicsComponent::OnPhysicsDestroyed();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		server->BroadcastTCP("ent_phys_destroy",p);
	}
}
void SPhysicsComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const {outTypeIndex = std::type_index(typeid(BasePhysicsComponent));}
void SPhysicsComponent::SetCollisionsEnabled(bool b)
{
	if(b == GetCollisionsEnabled())
		return;
	BasePhysicsComponent::SetCollisionsEnabled(b);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	NetPacket p {};
	p->Write<bool>(b);
	ent.SendNetEventTCP(m_netEvSetCollisionsEnabled,p);
}
void SPhysicsComponent::SetCollisionType(COLLISIONTYPE collisiontype)
{
	BasePhysicsComponent::SetCollisionType(collisiontype);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(!ent.IsShared())
		return;
	NetPacket p;
	nwm::write_entity(p,&ent);
	p->Write<unsigned char>(static_cast<unsigned char>(collisiontype));
	server->BroadcastTCP("ent_collisiontype",p);
}

void SPhysicsComponent::SetCollisionFilter(CollisionMask filterGroup,CollisionMask filterMask)
{
	BasePhysicsComponent::SetCollisionFilter(filterGroup,filterMask);
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	if(ent.IsShared())
	{
		NetPacket p;
		nwm::write_entity(p,&ent);
		p->Write<unsigned int>(static_cast<unsigned int>(filterGroup));
		p->Write<unsigned int>(static_cast<unsigned int>(filterMask));
		server->BroadcastTCP("ent_setcollisionfilter",p);
	}
}

void SPhysicsComponent::PostPhysicsSimulate()
{
	BasePhysicsComponent::PostPhysicsSimulate();
	if(GetPhysicsType() != PHYSICSTYPE::SOFTBODY)
		return;
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	auto *phys = GetPhysicsObject();
	auto *entCl = ent.GetClientsideEntity();
	auto pPhysComponent = (entCl != nullptr) ? entCl->GetPhysicsComponent() : util::WeakHandle<BasePhysicsComponent>{};
	auto *physCl = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	if(phys == nullptr || physCl == nullptr)
		return;
	// Only executed on listen servers; Serverside soft-body data will be transferred to client directly
	auto &colObjs = phys->GetCollisionObjects();
	auto &colObjsCl = physCl->GetCollisionObjects();
	auto numColObjs = umath::min(colObjs.size(),colObjsCl.size());
	for(auto i=decltype(numColObjs){0};i<numColObjs;++i)
	{
		auto &col = colObjs.at(i);
		auto &colCl = colObjsCl.at(i);
		if(col.IsValid() == false || colCl.IsValid() == false || col->IsSoftBody() == false || colCl->IsSoftBody() == false)
			continue;
		auto &sb = static_cast<PhysSoftBody&>(*col.get());
		auto &sbCl = static_cast<PhysSoftBody&>(*colCl.get());
		auto *btSb = sb.GetSoftBody();
		auto *btSbCl = sbCl.GetSoftBody();
		auto numNodes = umath::min(btSb->m_nodes.size(),btSbCl->m_nodes.size());
		for(auto j=decltype(numNodes){0};j<numNodes;++j)
		{
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
}
