// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.base_entity;

pragma::ComponentHandle<pragma::BaseEntityComponent> pragma::ecs::BaseEntity::AddNetworkedComponent(const std::string &name)
{
	auto c = FindComponent(name);
	if(c.valid())
		return c;
	return AddComponent(name);
}
std::shared_ptr<pragma::audio::ALSound> pragma::ecs::BaseEntity::CreateSound(const std::string &snd, audio::ALSoundType type)
{
	auto *sndC = static_cast<BaseSoundEmitterComponent *>(AddNetworkedComponent("sound_emitter").get());
	if(sndC == nullptr)
		return nullptr;
	return sndC->CreateSound(snd, type);
}
std::shared_ptr<pragma::audio::ALSound> pragma::ecs::BaseEntity::EmitSound(const std::string &snd, audio::ALSoundType type, float gain, float pitch)
{
	auto *sndC = static_cast<BaseSoundEmitterComponent *>(AddNetworkedComponent("sound_emitter").get());
	if(sndC == nullptr)
		return nullptr;
	return sndC->EmitSound(snd, type, {gain, pitch});
}

std::string pragma::ecs::BaseEntity::GetName() const
{
	auto nameC = GetNameComponent();
	return nameC.valid() ? nameC->GetName() : "";
}
void pragma::ecs::BaseEntity::SetName(const std::string &name)
{
	auto *nameC = static_cast<BaseNameComponent *>(AddNetworkedComponent("name").get());
	if(nameC == nullptr)
		return;
	nameC->SetName(name);
}

void pragma::ecs::BaseEntity::SetModel(const std::string &mdl)
{
	auto *mdlC = static_cast<BaseModelComponent *>(AddNetworkedComponent("model").get());
	if(mdlC == nullptr)
		return;
	mdlC->SetModel(mdl);
}
void pragma::ecs::BaseEntity::SetModel(const std::shared_ptr<asset::Model> &mdl)
{
	auto *mdlC = static_cast<BaseModelComponent *>(AddNetworkedComponent("model").get());
	if(mdlC == nullptr)
		return;
	mdlC->SetModel(mdl);
}
const std::shared_ptr<pragma::asset::Model> &pragma::ecs::BaseEntity::GetModel() const
{
	auto mdlC = GetModelComponent();
	static std::shared_ptr<asset::Model> nptr = nullptr;
	return mdlC ? mdlC->GetModel() : nptr;
}
std::string pragma::ecs::BaseEntity::GetModelName() const
{
	auto mdlC = GetModelComponent();
	return mdlC ? mdlC->GetModelName() : "";
}
std::optional<pragma::math::Transform> pragma::ecs::BaseEntity::GetAttachmentPose(uint32_t attId) const
{
	auto mdlC = GetModelComponent();
	math::Transform t {};
	if(mdlC) {
		Vector3 pos;
		Quat rot;
		if(mdlC->GetAttachment(attId, &pos, &rot))
			t = {pos, rot};
	}
	return t;
}
uint32_t pragma::ecs::BaseEntity::GetSkin() const
{
	auto mdlC = GetModelComponent();
	return mdlC ? mdlC->GetSkin() : 0;
}
void pragma::ecs::BaseEntity::SetSkin(uint32_t skin)
{
	auto mdlC = GetModelComponent();
	if(!mdlC)
		return;
	mdlC->SetSkin(skin);
}
uint32_t pragma::ecs::BaseEntity::GetBodyGroup(const std::string &name) const
{
	auto mdlC = GetModelComponent();
	if(!mdlC)
		return 0;
	auto &mdl = mdlC->GetModel();
	auto id = mdl ? mdl->GetBodyGroupId(name) : -1;
	return (id != -1) ? mdlC->GetBodyGroup(id) : 0;
}
void pragma::ecs::BaseEntity::SetBodyGroup(const std::string &name, uint32_t id)
{
	auto mdlC = GetModelComponent();
	if(!mdlC)
		return;
	mdlC->SetBodyGroup(name, id);
}

void pragma::ecs::BaseEntity::SetParent(BaseEntity *parent)
{
	if(!parent) {
		RemoveComponent("child");
		return;
	}
	auto childC = AddComponent("child");
	if(childC.expired())
		return;
	assert(m_childComponent != nullptr);
	if(!m_childComponent)
		return;
	m_childComponent->SetParent(*parent);
}

void pragma::ecs::BaseEntity::ClearParent() { SetParent(nullptr); }

pragma::ecs::BaseEntity *pragma::ecs::BaseEntity::GetParent() const { return m_childComponent ? m_childComponent->GetParentEntity() : nullptr; }

bool pragma::ecs::BaseEntity::HasParent() const { return GetParent() != nullptr; }
bool pragma::ecs::BaseEntity::HasChildren() const
{
	auto parentC = GetComponent<ParentComponent>();
	if(parentC.expired())
		return false;
	for(auto &hChild : parentC->GetChildren()) {
		if(hChild.valid())
			return true;
	}
	return false;
}

bool pragma::ecs::BaseEntity::IsChildOf(const BaseEntity &ent) const { return GetParent() == &ent; }
bool pragma::ecs::BaseEntity::IsDescendantOf(const BaseEntity &ent) const
{
	auto *parent = GetParent();
	while(parent) {
		if(parent == &ent)
			return true;
		parent = parent->GetParent();
	}
	return false;
}
bool pragma::ecs::BaseEntity::IsAncestorOf(const BaseEntity &ent) const { return ent.IsDescendantOf(*this); }
bool pragma::ecs::BaseEntity::IsParentOf(const BaseEntity &ent) const { return ent.IsChildOf(*this); }

pragma::physics::PhysObj *pragma::ecs::BaseEntity::GetPhysicsObject() const
{
	auto physC = GetPhysicsComponent();
	return physC ? physC->GetPhysicsObject() : nullptr;
}
pragma::physics::PhysObj *pragma::ecs::BaseEntity::InitializePhysics(physics::PhysicsType type)
{
	auto *physC = static_cast<BasePhysicsComponent *>(AddNetworkedComponent("physics").get());
	if(physC == nullptr)
		return nullptr;
	return physC->InitializePhysics(type);
}
void pragma::ecs::BaseEntity::DestroyPhysicsObject()
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return;
	physC->DestroyPhysicsObject();
}
void pragma::ecs::BaseEntity::DropToFloor()
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return;
	physC->DropToFloor();
}
std::pair<Vector3, Vector3> pragma::ecs::BaseEntity::GetCollisionBounds() const
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return {Vector3 {}, Vector3 {}};
	Vector3 min, max;
	physC->GetCollisionBounds(&min, &max);
	return {min, max};
}
void pragma::ecs::BaseEntity::SetCollisionFilterMask(physics::CollisionMask filterMask)
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return;
	physC->SetCollisionFilterMask(filterMask);
}
void pragma::ecs::BaseEntity::SetCollisionFilterGroup(physics::CollisionMask filterGroup)
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return;
	physC->SetCollisionFilter(filterGroup);
}
pragma::physics::CollisionMask pragma::ecs::BaseEntity::GetCollisionFilterGroup() const
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return physics::CollisionMask::None;
	return physC->GetCollisionFilter();
}
pragma::physics::CollisionMask pragma::ecs::BaseEntity::GetCollisionFilterMask() const
{
	auto physC = GetPhysicsComponent();
	if(!physC)
		return physics::CollisionMask::None;
	return physC->GetCollisionFilterMask();
}

Vector3 pragma::ecs::BaseEntity::GetForward() const
{
	auto trC = GetTransformComponent();
	return trC ? trC->GetForward() : uvec::PRM_FORWARD;
}
Vector3 pragma::ecs::BaseEntity::GetUp() const
{
	auto trC = GetTransformComponent();
	return trC ? trC->GetUp() : uvec::PRM_UP;
}
Vector3 pragma::ecs::BaseEntity::GetRight() const
{
	auto trC = GetTransformComponent();
	return trC ? trC->GetRight() : uvec::PRM_RIGHT;
}

void pragma::ecs::BaseEntity::Input(const std::string &input, BaseEntity *activator, BaseEntity *caller, const std::string &data)
{
	auto *ioC = static_cast<BaseIOComponent *>(AddNetworkedComponent("io").get());
	if(ioC == nullptr)
		return;
	ioC->Input(input, activator, caller, data);
}

uint16_t pragma::ecs::BaseEntity::GetHealth() const
{
	auto healthC = FindComponent("health");
	return healthC.valid() ? static_cast<BaseHealthComponent &>(*healthC).GetHealth() : 0;
}
uint16_t pragma::ecs::BaseEntity::GetMaxHealth() const
{
	auto healthC = FindComponent("health");
	return healthC.valid() ? static_cast<BaseHealthComponent &>(*healthC).GetMaxHealth() : 0;
}
void pragma::ecs::BaseEntity::SetHealth(uint16_t health)
{
	auto *healthC = static_cast<BaseHealthComponent *>(AddNetworkedComponent("health").get());
	if(healthC == nullptr)
		return;
	healthC->SetHealth(health);
}
void pragma::ecs::BaseEntity::SetMaxHealth(uint16_t maxHealth)
{
	auto *healthC = static_cast<BaseHealthComponent *>(AddNetworkedComponent("health").get());
	if(healthC == nullptr)
		return;
	healthC->SetMaxHealth(maxHealth);
}

void pragma::ecs::BaseEntity::SetVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<VelocityComponent *>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->SetVelocity(vel);
}
void pragma::ecs::BaseEntity::AddVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<VelocityComponent *>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->AddVelocity(vel);
}
Vector3 pragma::ecs::BaseEntity::GetVelocity() const
{
	auto velC = GetComponent<VelocityComponent>();
	return velC.valid() ? velC->GetVelocity() : Vector3 {};
}
void pragma::ecs::BaseEntity::SetAngularVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<VelocityComponent *>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->SetAngularVelocity(vel);
}
void pragma::ecs::BaseEntity::AddAngularVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<VelocityComponent *>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->AddAngularVelocity(vel);
}
Vector3 pragma::ecs::BaseEntity::GetAngularVelocity() const
{
	auto velC = GetComponent<VelocityComponent>();
	return velC.valid() ? velC->GetAngularVelocity() : Vector3 {};
}

void pragma::ecs::BaseEntity::PlayAnimation(int32_t animation, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return;
	animC->PlayAnimation(animation, flags);
}
void pragma::ecs::BaseEntity::PlayLayeredAnimation(int32_t slot, int32_t animation, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return;
	animC->PlayLayeredAnimation(slot, animation, flags);
}
bool pragma::ecs::BaseEntity::PlayActivity(Activity activity, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayActivity(activity, flags);
}
bool pragma::ecs::BaseEntity::PlayLayeredActivity(int32_t slot, Activity activity, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayLayeredActivity(slot, activity, flags);
}
bool pragma::ecs::BaseEntity::PlayLayeredAnimation(int32_t slot, std::string animation, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayLayeredAnimation(slot, animation, flags);
}
void pragma::ecs::BaseEntity::StopLayeredAnimation(int slot)
{
	auto animC = GetAnimatedComponent();
	if(animC.expired())
		return;
	animC->StopLayeredAnimation(slot);
}
bool pragma::ecs::BaseEntity::PlayAnimation(const std::string &animation, FPlayAnim flags)
{
	auto *animC = static_cast<BaseAnimatedComponent *>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayAnimation(animation, flags);
}
int32_t pragma::ecs::BaseEntity::GetAnimation() const
{
	auto animC = GetAnimatedComponent();
	return animC.valid() ? animC->GetAnimation() : -1;
}
pragma::Activity pragma::ecs::BaseEntity::GetActivity() const
{
	auto animC = GetAnimatedComponent();
	return animC.valid() ? animC->GetActivity() : Activity::Invalid;
}

void pragma::ecs::BaseEntity::TakeDamage(game::DamageInfo &info)
{
	auto *dmgC = static_cast<DamageableComponent *>(AddNetworkedComponent("damageable").get());
	if(dmgC == nullptr)
		return;
	dmgC->TakeDamage(info);
}
