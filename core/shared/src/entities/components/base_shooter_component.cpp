/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_shooter_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/physics/raytraces.h"
#include "pragma/lua/classes/ldef_vector.h"
#include <sharedutils/netpacket.hpp>

using namespace pragma;

ComponentEventId BaseShooterComponent::EVENT_ON_FIRE_BULLETS = INVALID_COMPONENT_ID;
ComponentEventId BaseShooterComponent::EVENT_ON_BULLETS_FIRED = INVALID_COMPONENT_ID;
void BaseShooterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_FIRE_BULLETS = componentManager.RegisterEvent("ON_FIRE_BULLETS");
	EVENT_ON_BULLETS_FIRED = componentManager.RegisterEvent("ON_BULLETS_FIRED");
}
BaseShooterComponent::BaseShooterComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void BaseShooterComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvFireBullets = SetupNetEvent("fire_bullets");

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}

void BaseShooterComponent::ReceiveBulletEvent(NetPacket &packet,pragma::BasePlayerComponent *pl)
{
	m_nextBullet = std::unique_ptr<NextBulletInfo>(new NextBulletInfo);
	auto numBullets = packet->Read<uint32_t>();
	m_nextBullet->destinations.reserve(numBullets);
	for(auto i=decltype(numBullets){0};i<numBullets;++i)
		m_nextBullet->destinations.push_back(packet->Read<Vector3>());
	if(pl != nullptr)
		m_nextBullet->source = pl->GetEntity().GetHandle();
}

std::vector<Vector3> BaseShooterComponent::GetBulletDestinations(const Vector3 &origin,const Vector3 &dir,const BulletInfo &bulletInfo)
{
	std::vector<Vector3> destPositions;
	destPositions.reserve(bulletInfo.bulletCount);
	for(auto i=decltype(bulletInfo.bulletCount){0};i<bulletInfo.bulletCount;++i)
	{
		auto randSpread = EulerAngles(umath::random(-bulletInfo.spread.p,bulletInfo.spread.p),umath::random(-bulletInfo.spread.y,bulletInfo.spread.y),0);
		auto bulletDir = dir;
		uvec::rotate(&bulletDir,randSpread);

		destPositions.push_back(origin +bulletDir *bulletInfo.distance);
	}
	return destPositions;
}

void BaseShooterComponent::OnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent.valid())
	{
		bulletOrigin = pTrComponent->GetEyePosition();
		bulletDir = pTrComponent->GetForward();
	}
	if(effectsOrigin != nullptr)
		*effectsOrigin = bulletOrigin;

	CEOnFireBullets evData {bulletInfo,bulletOrigin,bulletDir,effectsOrigin};
	BroadcastEvent(EVENT_ON_FIRE_BULLETS,evData);
}

RayCastHitType BaseShooterComponent::OnBulletHit(const BulletInfo &bulletInfo,const TraceData &data,PhysObj &phys,physics::ICollisionObject &col) {return RayCastHitType::Block;}

void BaseShooterComponent::GetBulletTraceData(const BulletInfo &bulletInfo,TraceData &data) const
{
	auto *attacker = bulletInfo.hAttacker.get();
	auto *inflictor = bulletInfo.hInflictor.get();
	auto *entSrc = (attacker != nullptr) ? attacker : (inflictor != nullptr) ? inflictor : &GetEntity();
	data.SetCollisionFilterMask(CollisionMask::AllHitbox &~CollisionMask::Trigger); // Let everything pass (Except specific filters below)
	data.SetFilter([this,&data,attacker,inflictor,&bulletInfo](pragma::physics::IShape &shape,pragma::physics::IRigidBody &body) -> RayCastHitType {
		auto *phys = body.GetPhysObj();
		auto *ent = phys ? phys->GetOwner() : nullptr;
		if(ent == nullptr || &ent->GetEntity() == &GetEntity() || &ent->GetEntity() == attacker || &ent->GetEntity() == inflictor) // Attacker can't shoot themselves or the inflictor
			return RayCastHitType::None;
		auto filterGroup = phys->GetCollisionFilter();
		auto mdlComponent = ent->GetEntity().GetModelComponent();
		if(mdlComponent.valid() && mdlComponent->GetHitboxCount() > 0 && (filterGroup &CollisionMask::NPC) != CollisionMask::None || (filterGroup &CollisionMask::Player) != CollisionMask::None) // Filter out player and NPC collision objects, since we only want to check their hitboxes
			return RayCastHitType::None;
		return const_cast<BaseShooterComponent*>(this)->OnBulletHit(bulletInfo,data,*phys,body);
	});
	auto physComponent = GetEntity().GetPhysicsComponent();
	auto filterGroup = CollisionMask::None;
	if(physComponent.valid())
		filterGroup = physComponent->GetCollisionFilter();
	filterGroup |= CollisionMask::Water | CollisionMask::WaterSurface | CollisionMask::PlayerHitbox | CollisionMask::NPCHitbox;
	data.SetCollisionFilterGroup(filterGroup);
}

//////////////

CEOnFireBullets::CEOnFireBullets(const BulletInfo &bulletInfo,Vector3 &bulletOrigin,Vector3 &bulletDir,Vector3 *effectsOrigin)
	: bulletInfo{bulletInfo},bulletOrigin{bulletOrigin},bulletDir{bulletDir},effectsOrigin{effectsOrigin}
{}
void CEOnFireBullets::PushArguments(lua_State *l)
{
	Lua::Push<BulletInfo*>(l,&const_cast<BulletInfo&>(bulletInfo));
	Lua::Push<Vector3>(l,bulletOrigin);
	Lua::Push<Vector3>(l,bulletDir);
	if(effectsOrigin != nullptr)
		Lua::Push<Vector3>(l,*effectsOrigin);
	else
		Lua::PushNil(l);
}
uint32_t CEOnFireBullets::GetReturnCount() {return 3;}
void CEOnFireBullets::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-3))
		bulletOrigin = *Lua::CheckVector(l,-3);
	if(Lua::IsSet(l,-2))
		bulletDir = *Lua::CheckVector(l,-2);
	if(Lua::IsSet(l,-1) && effectsOrigin != nullptr)
		*effectsOrigin = *Lua::CheckVector(l,-1);
}

//////////////

CEOnBulletsFired::CEOnBulletsFired(const BulletInfo &bulletInfo,const std::vector<TraceResult> &hitTargets)
	: bulletInfo{bulletInfo},hitTargets{hitTargets}
{}
void CEOnBulletsFired::PushArguments(lua_State *l)
{
	Lua::Push<BulletInfo*>(l,&const_cast<BulletInfo&>(bulletInfo));

	auto t = Lua::CreateTable(l);
	for(auto i=decltype(hitTargets.size()){0};i<hitTargets.size();++i)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<TraceResult*>(l,const_cast<TraceResult*>(&hitTargets.at(i)));
		Lua::SetTableValue(l,t);
	}
}

