// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_weapon;

using namespace pragma;

void BaseWeaponComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseWeaponComponent::EVENT_ON_DEPLOY = registerEvent("ON_DEPLOY", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_HOLSTER = registerEvent("ON_HOLSTER", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_PRIMARY_ATTACK = registerEvent("ON_PRIMARY_ATTACK", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_SECONDARY_ATTACK = registerEvent("ON_SECONDARY_ATTACK", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_TERTIARY_ATTACK = registerEvent("ON_TERTIARY_ATTACK", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_ATTACK4 = registerEvent("ON_ATTACK4", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_END_PRIMARY_ATTACK = registerEvent("ON_END_PRIMARY_ATTACK", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_END_SECONDARY_ATTACK = registerEvent("ON_END_SECONDARY_ATTACK", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_RELOAD = registerEvent("ON_RELOAD", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_PRIMARY_CLIP_SIZE_CHANGED = registerEvent("ON_PRIMARY_CLIP_SIZE_CHANGED", ComponentEventInfo::Type::Broadcast);
	baseWeaponComponent::EVENT_ON_SECONDARY_CLIP_SIZE_CHANGED = registerEvent("ON_SECONDARY_CLIP_SIZE_CHANGED", ComponentEventInfo::Type::Broadcast);
}
BaseWeaponComponent::BaseWeaponComponent(ecs::BaseEntity &ent)
    : BaseEntityComponent(ent), m_clipPrimary(util::UInt16Property::Create(0)), m_clipSecondary(util::UInt16Property::Create(0)), m_maxPrimaryClipSize(util::UInt16Property::Create(30)), m_maxSecondaryClipSize(util::UInt16Property::Create(10)),
      m_ammoPrimary(util::UInt32Property::Create(0)), m_ammoSecondary(util::UInt32Property::Create(0))
{
}

bool BaseWeaponComponent::IsDeployed() const { return m_bDeployed; }

void BaseWeaponComponent::SetNextPrimaryAttack(float t) { m_tNextPrimaryAttack = t; }
void BaseWeaponComponent::SetNextSecondaryAttack(float t) { m_tNextSecondaryAttack = t; }
void BaseWeaponComponent::SetNextAttack(float t)
{
	SetNextPrimaryAttack(t);
	SetNextSecondaryAttack(t);
}

void BaseWeaponComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(!pPhysComponent)
		return;
	pPhysComponent->AddCollisionFilter(physics::CollisionMask::Item);
	pPhysComponent->SetCollisionFilterGroup(pPhysComponent->GetCollisionFilter() | physics::CollisionMask::Item);
}

void BaseWeaponComponent::OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if(owner != nullptr) {
		auto pTrComponent = owner->GetTransformComponent();
		if(pTrComponent) {
			bulletOrigin = pTrComponent->GetEyePosition(); // TODO ShootPos?
			bulletDir = pTrComponent->GetForward();        // TODO View Forward
		}
	}
	else if(m_attMuzzle != -1) {
		auto rot = uquat::identity();
		auto &ent = GetEntity();
		auto mdlC = ent.GetModelComponent();
		if(mdlC && mdlC->GetAttachment(m_attMuzzle, static_cast<Vector3 *>(nullptr), &rot) == true) {
			auto pTrComponent = ent.GetTransformComponent();
			if(pTrComponent)
				pTrComponent->LocalToWorld(&rot);
			bulletDir = uquat::forward(rot);
		}
	}
}

void BaseWeaponComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pOwnerComponent = dynamic_cast<BaseOwnableComponent *>(&component);
	if(pOwnerComponent != nullptr)
		m_whOwnerComponent = pOwnerComponent->GetHandle<BaseOwnableComponent>();
}

void BaseWeaponComponent::UpdateTickPolicy()
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	auto shouldTick = (m_bInAttack1 == true || m_bInAttack2 == true) && owner != nullptr;
	SetTickPolicy(shouldTick ? TickPolicy::Always : TickPolicy::Never);
}

void BaseWeaponComponent::OnTick(double)
{
	auto *owner = m_whOwnerComponent.valid() ? m_whOwnerComponent->GetOwner() : nullptr;
	if((m_bInAttack1 == true || m_bInAttack2 == true) && owner != nullptr) {
		if(owner->IsPlayer()) {
			auto plComponent = owner->GetPlayerComponent();
			auto *inputC = plComponent->GetActionInputController();
			if(m_bInAttack1 == true) {
				if(inputC && inputC->GetActionInput(Action::Attack) == true) {
					if(CanPrimaryAttack())
						PrimaryAttack();
				}
				else {
					m_bInAttack1 = false;
					UpdateTickPolicy();
					EndPrimaryAttack();
				}
			}
			if(m_bInAttack2 == true) {
				if(inputC && inputC->GetActionInput(Action::Attack2) == true) {
					if(CanSecondaryAttack())
						SecondaryAttack();
				}
				else {
					m_bInAttack2 = true;
					UpdateTickPolicy();
					EndSecondaryAttack();
				}
			}
		}
		else {
			if(m_bInAttack1 == true && CanPrimaryAttack())
				PrimaryAttack();
			if(m_bInAttack2 == true && CanSecondaryAttack())
				SecondaryAttack();
		}
	}
}

void BaseWeaponComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	EndAttack();
}

void BaseWeaponComponent::EndAttack()
{
	if(m_bInAttack1 == true)
		EndPrimaryAttack();
	if(m_bInAttack2 == true)
		EndSecondaryAttack();
}

void BaseWeaponComponent::EndPrimaryAttack()
{
	m_bInAttack1 = false;
	UpdateTickPolicy();

	BroadcastEvent(baseWeaponComponent::EVENT_ON_END_PRIMARY_ATTACK);
}
void BaseWeaponComponent::EndSecondaryAttack()
{
	m_bInAttack2 = false;
	UpdateTickPolicy();

	BroadcastEvent(baseWeaponComponent::EVENT_ON_END_SECONDARY_ATTACK);
}

void BaseWeaponComponent::SetAutomaticPrimary(bool b)
{
	m_bAutomaticPrimary = b;
	if(b == false) {
		m_bInAttack1 = false;
		UpdateTickPolicy();
	}
}
void BaseWeaponComponent::SetAutomaticSecondary(bool b)
{
	m_bAutomaticSecondary = b;
	if(b == false) {
		m_bInAttack2 = false;
		UpdateTickPolicy();
	}
}
bool BaseWeaponComponent::IsAutomaticPrimary() const { return m_bAutomaticPrimary; }
bool BaseWeaponComponent::IsAutomaticSecondary() const { return m_bAutomaticSecondary; }

bool BaseWeaponComponent::CanPrimaryAttack() const
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto &t = game->CurTime();
	return (t >= m_tNextPrimaryAttack) ? true : false;
}
bool BaseWeaponComponent::CanSecondaryAttack() const
{
	auto &ent = GetEntity();
	auto *nw = ent.GetNetworkState();
	auto *game = nw->GetGameState();
	auto &t = game->CurTime();
	return (t >= m_tNextSecondaryAttack) ? true : false;
}

void BaseWeaponComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		m_attMuzzle = mdlComponent ? mdlComponent->LookupAttachment("muzzle") : -1;
	});
	BindEventUnhandled(ecs::baseShooterComponent::EVENT_ON_FIRE_BULLETS, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &evDataOnFireBullets = static_cast<ecs::events::CEOnFireBullets &>(evData.get());
		OnFireBullets(evDataOnFireBullets.bulletInfo, evDataOnFireBullets.bulletOrigin, evDataOnFireBullets.bulletDir, evDataOnFireBullets.effectsOrigin);
	});
	BindEventUnhandled(baseOwnableComponent::EVENT_ON_OWNER_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		auto &ownerChangedData = static_cast<CEOnOwnerChanged &>(evData.get());
		auto &ent = GetEntity();
		auto ptrPhysComponent = ent.GetPhysicsComponent();
		if(ptrPhysComponent)
			ptrPhysComponent->DestroyPhysicsObject();
		if(ownerChangedData.newOwner == nullptr)
			EndAttack();
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	ent.AddComponent("ownable");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent *>(whRenderComponent.get())->SetCastShadows(true);
}

BaseOwnableComponent *BaseWeaponComponent::GetOwnerComponent() { return m_whOwnerComponent.get(); }

void BaseWeaponComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

void BaseWeaponComponent::Deploy()
{
	m_bDeployed = true;
	auto animComponent = GetEntity().GetAnimatedComponent();
	if(animComponent.valid())
		animComponent->PlayActivity(Activity::VmIdle);

	BroadcastEvent(baseWeaponComponent::EVENT_ON_DEPLOY);
}

void BaseWeaponComponent::Holster()
{
	EndAttack();
	m_bDeployed = false;

	BroadcastEvent(baseWeaponComponent::EVENT_ON_HOLSTER);
}

void BaseWeaponComponent::PrimaryAttack()
{
	if(IsAutomaticPrimary()) {
		m_bInAttack1 = true;
		UpdateTickPolicy();
	}

	BroadcastEvent(baseWeaponComponent::EVENT_ON_PRIMARY_ATTACK);
}

void BaseWeaponComponent::SecondaryAttack()
{
	if(IsAutomaticSecondary()) {
		m_bInAttack2 = true;
		UpdateTickPolicy();
	}

	BroadcastEvent(baseWeaponComponent::EVENT_ON_SECONDARY_ATTACK);
}

void BaseWeaponComponent::TertiaryAttack() { BroadcastEvent(baseWeaponComponent::EVENT_ON_TERTIARY_ATTACK); }

void BaseWeaponComponent::Attack4() { BroadcastEvent(baseWeaponComponent::EVENT_ON_ATTACK4); }

void BaseWeaponComponent::Reload() { BroadcastEvent(baseWeaponComponent::EVENT_ON_RELOAD); }

//////////////

CEOnOwnerChanged::CEOnOwnerChanged(ecs::BaseEntity *oldOwner, ecs::BaseEntity *newOwner) : oldOwner {oldOwner}, newOwner {newOwner} {}
void CEOnOwnerChanged::PushArguments(lua::State *l)
{
	if(oldOwner != nullptr)
		oldOwner->GetLuaObject().push(l);
	else
		Lua::PushNil(l);

	if(newOwner != nullptr)
		newOwner->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

//////////////

CEOnClipSizeChanged::CEOnClipSizeChanged(UInt16 oldClipSize, UInt16 newClipSize) : oldClipSize {oldClipSize}, newClipSize {newClipSize} {}
void CEOnClipSizeChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, oldClipSize);
	Lua::PushInt(l, newClipSize);
}
