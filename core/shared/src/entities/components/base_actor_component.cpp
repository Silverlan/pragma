#include "stdafx_shared.h"
#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physshape.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_observable_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/model/model.h"
#include <pragma/physics/movetypes.h>

using namespace pragma;

#pragma optimize("",off)
BaseActorComponent::HitboxData::HitboxData(uint32_t _boneId,const Vector3 &_offset)
	: boneId(_boneId),offset(_offset)
{}

//////////////////

ComponentEventId BaseActorComponent::EVENT_ON_KILLED = INVALID_COMPONENT_ID;
ComponentEventId BaseActorComponent::EVENT_ON_RESPAWN = INVALID_COMPONENT_ID;
ComponentEventId BaseActorComponent::EVENT_ON_DEATH = INVALID_COMPONENT_ID;
void BaseActorComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_KILLED = componentManager.RegisterEvent("ON_KILLED");
	EVENT_ON_RESPAWN = componentManager.RegisterEvent("ON_RESPAWN");
	EVENT_ON_DEATH = componentManager.RegisterEvent("ON_DEATH");
}
BaseActorComponent::BaseActorComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_bAlive(true),m_bFrozen(util::BoolProperty::Create(false))
{}

void BaseActorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		InitializeMoveController();
		return util::EventReply::Unhandled;
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPhysicsInitialized();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_DESTROYED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPhysicsDestroyed();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_UPDATED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		PhysicsUpdate(0.0 /* unused */);
	});

	auto &ent = GetEntity();
	ent.AddComponent("gravity");
	ent.AddComponent("name");
	ent.AddComponent("flammable");
	ent.AddComponent("health");
	auto whObservableComponent = ent.AddComponent("observable");
	if(whObservableComponent.valid())
	{
		auto *pObservableComponent = static_cast<BaseObservableComponent*>(whObservableComponent.get());
		pObservableComponent->SetFirstPersonObserverOffsetEnabled(false);
		pObservableComponent->SetThirdPersonObserverOffsetEnabled(true);
	}
	ent.AddComponent("submergible");
	ent.AddComponent("physics");
	ent.AddComponent("transform");
	ent.AddComponent("sound_emitter");
	ent.AddComponent("velocity");
	ent.AddComponent("damageable");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);

	m_netEvSetFrozen = SetupNetEvent("set_frozen");
}

void BaseActorComponent::SetMoveController(const std::string &moveController)
{
	m_moveControllerName = moveController;
	InitializeMoveController();
}
int32_t BaseActorComponent::GetMoveController() const {return m_moveController;}
void BaseActorComponent::InitializeMoveController()
{
	m_moveController = -1;
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto mdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(mdl == nullptr)
		return;
	m_moveController = mdl->LookupBlendController(m_moveControllerName);
}

void BaseActorComponent::UpdateMoveController()
{
	if(m_moveController == -1)
		return;
	auto &ent = GetEntity();
	auto animComponent = ent.GetAnimatedComponent();
	if(animComponent.expired())
		return;
	auto pTrComponent = ent.GetTransformComponent();
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	auto vel = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3{};
	auto l = uvec::length_sqr(vel);
	if(l > 0.f)
	{
		auto dirMove = uvec::get_normal(vel);
		auto dir = pTrComponent.valid() ? pTrComponent->GetForward() : uvec::FORWARD;
		float yawMove = uvec::get_yaw(dirMove);
		float yawDir = uvec::get_yaw(dir);
		float yawOffset = umath::get_angle_difference(yawDir,yawMove);
		yawOffset = umath::normalize_angle(yawOffset,0);
		//float moveYaw = GetBlendController(blendController);
		//yawOffset = Math::ApproachAngle(moveYaw,yawOffset,1.f);
		animComponent->SetBlendController(m_moveController,CInt32(yawOffset));
	}
}

const util::PBoolProperty &BaseActorComponent::GetFrozenProperty() const {return m_bFrozen;}
bool BaseActorComponent::IsFrozen() const {return *m_bFrozen;}
bool BaseActorComponent::IsAlive() const {return m_bAlive;}
bool BaseActorComponent::IsDead() const {return !IsAlive();}
void BaseActorComponent::SetFrozen(bool b) {*m_bFrozen = b;}

void BaseActorComponent::Ragdolize()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.expired())
		return;
	auto *phys = pPhysComponent->GetPhysicsObject();
	if(phys != nullptr && pPhysComponent->GetPhysicsType() == PHYSICSTYPE::DYNAMIC)
		return;
	pPhysComponent->SetMoveType(MOVETYPE::PHYSICS);
	phys = pPhysComponent->InitializePhysics(PHYSICSTYPE::DYNAMIC);
	if(phys == nullptr)
		return;
}

void BaseActorComponent::OnDeath(DamageInfo *dmgInfo)
{
	auto evOnDeath = CEOnCharacterKilled{dmgInfo};
	if(BroadcastEvent(EVENT_ON_DEATH,evOnDeath) == util::EventReply::Handled)
		return;
	
	Ragdolize();
}

void BaseActorComponent::Kill(DamageInfo *dmgInfo)
{
	if(!IsAlive())
		return;
	m_bAlive = false;
	OnDeath(dmgInfo);

	auto evOnKilled = CEOnCharacterKilled{dmgInfo};
	BroadcastEvent(EVENT_ON_KILLED,evOnKilled);
}

void BaseActorComponent::Respawn()
{
	m_bAlive = true;
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->SetMoveType(MOVETYPE::WALK);
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent.valid())
		pTrComponent->SetOrientation(uquat::identity());
	auto pVelComponent = ent.GetComponent<pragma::VelocityComponent>();
	if(pVelComponent.valid())
	{
		pVelComponent->SetVelocity({});
		pVelComponent->SetAngularVelocity({});
	}
	BroadcastEvent(EVENT_ON_RESPAWN);
}

void BaseActorComponent::OnPhysicsInitialized()
{
	m_hitboxData.clear();
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl != nullptr)
	{
		auto *nw = ent.GetNetworkState();
		auto *game = nw->GetGameState();
		auto *physEnv = game->GetPhysicsEnvironment();
		auto hitboxBones = hMdl->GetHitboxBones();
		auto pTrComponent = ent.GetTransformComponent();
		auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};

		auto &hitboxes = hMdl->GetHitboxes();
		std::vector<PhysCollisionObject*> physHitboxes;
		physHitboxes.reserve(hitboxes.size());
		m_hitboxData.reserve(hitboxes.size());
		for(auto &it : hitboxes)
		{
			auto &hb = it.second;
			auto min = hb.min *scale;
			auto max = hb.max *scale;
			auto extents = (max -min) *0.5f;
			auto *col = physEnv->CreateGhostObject(physEnv->CreateBoxShape(extents));
			col->SetCollisionFlags(col->GetCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE); // Disable collisions
			physHitboxes.push_back(col);

			// Physics object origin is in the center of the hitbox bounds, but the origin of the actual hitbox may not be; We need to calculate the offset between them and rotate it later when updating (See BaseActorComponent::UpdateHitboxPhysics)
			auto offset = max -extents; // Offset between bone origin and physics object origin
			m_hitboxData.push_back({it.first,offset});
		}
		m_physHitboxes = std::make_unique<PhysObj>(this,&physHitboxes);
		auto collisionMask = ent.IsPlayer() ? CollisionMask::PlayerHitbox : CollisionMask::NPCHitbox;
		m_physHitboxes->SetCollisionFilter(collisionMask,collisionMask); // Required for raytraces
		m_physHitboxes->Spawn();
	}
}
bool BaseActorComponent::FindHitgroup(const PhysCollisionObject &phys,HitGroup &hitgroup) const
{
	if(m_physHitboxes == nullptr)
		return false;
	auto &colObjs = m_physHitboxes->GetCollisionObjects();
	auto it = std::find_if(colObjs.begin(),colObjs.end(),[&phys](const PhysCollisionObjectHandle &hPhys) {
		return (hPhys.get() == &phys) ? true : false;
	});
	if(it == colObjs.end())
		return false;
	auto idx = it -colObjs.begin();
	if(idx >= m_hitboxData.size())
		return false;
	auto &hitboxData = m_hitboxData[idx];
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr)
		return false;
	auto &hitboxes = hMdl->GetHitboxes();
	auto itHitbox = hitboxes.find(hitboxData.boneId);
	if(itHitbox == hitboxes.end())
		return false;
	hitgroup = itHitbox->second.group;
	return true;
}
void BaseActorComponent::PhysicsUpdate(double)
{
	UpdateHitboxPhysics();
}
PhysObjHandle BaseActorComponent::GetHitboxPhysicsObject() const {return (m_physHitboxes == nullptr) ? PhysObjHandle{} : m_physHitboxes->GetHandle();}
void BaseActorComponent::UpdateHitboxPhysics()
{
	if(m_physHitboxes == nullptr)
		return;
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	if(mdlComponent.expired())
		return;
	auto &colObjs = m_physHitboxes->GetCollisionObjects();
	auto numColObjs = colObjs.size();
	for(auto i=decltype(m_hitboxData.size()){0};i<m_hitboxData.size();++i)
	{
		if(i >= numColObjs)
			break;
		auto &hitboxInfo = m_hitboxData[i];
		auto *col = static_cast<PhysGhostObject*>(colObjs[i].get());
		if(col != nullptr)
		{
			Vector3 min,max,origin;
			Quat rot;
			if(mdlComponent->GetHitboxBounds(hitboxInfo.boneId,min,max,origin,rot) == true)
			{
				auto offset = hitboxInfo.offset;
				offset = offset *uquat::get_inverse(rot);
				origin += offset;

				col->SetPos(origin);
				col->SetRotation(rot);
			}
		}
	}
}
void BaseActorComponent::OnPhysicsDestroyed()
{
	m_physHitboxes = nullptr;
	m_hitboxData.clear();
}

///////////////

CEOnCharacterKilled::CEOnCharacterKilled(DamageInfo *damageInfo)
	: damageInfo(damageInfo)
{}
void CEOnCharacterKilled::PushArguments(lua_State *l)
{
	if(damageInfo != nullptr)
		Lua::Push<boost::reference_wrapper<DamageInfo>>(l,boost::reference_wrapper<DamageInfo>(*damageInfo));
	else
		Lua::PushNil(l);
}
#pragma optimize("",on)
