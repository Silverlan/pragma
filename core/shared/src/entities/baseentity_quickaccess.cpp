#include "pragma/entities/baseentity.h"
#include "pragma/physics/transform.hpp"
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_health_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_attachable_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/model/model.h"

util::WeakHandle<pragma::BaseEntityComponent> BaseEntity::AddNetworkedComponent(const std::string &name)
{
	auto c = FindComponent(name);
	if(c.valid())
		return c;
	return AddComponent(name);
}
std::shared_ptr<ALSound> BaseEntity::CreateSound(const std::string &snd,ALSoundType type)
{
	auto *sndC = static_cast<pragma::BaseSoundEmitterComponent*>(AddNetworkedComponent("sound_emitter").get());
	if(sndC == nullptr)
		return nullptr;
	return sndC->CreateSound(snd,type);
}
std::shared_ptr<ALSound> BaseEntity::EmitSound(const std::string &snd,ALSoundType type,float gain,float pitch)
{
	auto *sndC = static_cast<pragma::BaseSoundEmitterComponent*>(AddNetworkedComponent("sound_emitter").get());
	if(sndC == nullptr)
		return nullptr;
	return sndC->EmitSound(snd,type,gain,pitch);
}

std::string BaseEntity::GetName() const
{
	auto nameC = GetNameComponent();
	return nameC.valid() ? nameC->GetName() : "";
}
void BaseEntity::SetName(const std::string &name)
{
	auto *nameC = static_cast<pragma::BaseNameComponent*>(AddNetworkedComponent("name").get());
	if(nameC == nullptr)
		return;
	nameC->SetName(name);
}

void BaseEntity::SetModel(const std::string &mdl)
{
	auto *mdlC = static_cast<pragma::BaseModelComponent*>(AddNetworkedComponent("model").get());
	if(mdlC == nullptr)
		return;
	mdlC->SetModel(mdl);
}
void BaseEntity::SetModel(const std::shared_ptr<Model> &mdl)
{
	auto *mdlC = static_cast<pragma::BaseModelComponent*>(AddNetworkedComponent("model").get());
	if(mdlC == nullptr)
		return;
	mdlC->SetModel(mdl);
}
std::shared_ptr<Model> BaseEntity::GetModel() const
{
	auto mdlC = GetModelComponent();
	return mdlC.valid() ? mdlC->GetModel() : nullptr;
}
std::string BaseEntity::GetModelName() const
{
	auto mdlC = GetModelComponent();
	return mdlC.valid() ? mdlC->GetModelName() : "";
}
std::optional<pragma::physics::Transform> BaseEntity::GetAttachmentPose(uint32_t attId) const
{
	auto mdlC = GetModelComponent();
	pragma::physics::Transform t {};
	if(mdlC.valid())
	{
		Vector3 pos;
		Quat rot;
		if(mdlC->GetAttachment(attId,&pos,&rot))
			t = {pos,rot};
	}
	return t;
}
uint32_t BaseEntity::GetSkin() const
{
	auto mdlC = GetModelComponent();
	return mdlC.valid() ? mdlC->GetSkin() : 0;
}
void BaseEntity::SetSkin(uint32_t skin)
{
	auto mdlC = GetModelComponent();
	if(mdlC.expired())
		return;
	mdlC->SetSkin(skin);
}
uint32_t BaseEntity::GetBodyGroup(const std::string &name) const
{
	auto mdlC = GetModelComponent();
	if(mdlC.expired())
		return 0;
	auto &mdl = mdlC->GetModel();
	auto id = mdl ? mdl->GetBodyGroupId(name) : -1;
	return (id != -1) ? mdlC->GetBodyGroup(id) : 0;
}
void BaseEntity::SetBodyGroup(const std::string &name,uint32_t id)
{
	auto mdlC = GetModelComponent();
	if(mdlC.expired())
		return;
	mdlC->SetBodyGroup(name,id);
}

void BaseEntity::SetParent(BaseEntity *parent)
{
	auto *attC = static_cast<pragma::BaseAttachableComponent*>(AddNetworkedComponent("attachable").get());
	if(attC == nullptr)
		return;
	attC->AttachToEntity(parent);
}

void BaseEntity::ClearParent() {SetParent(nullptr);}

pragma::BaseParentComponent *BaseEntity::GetParent() const
{
	auto attC = FindComponent("attachable");
	if(attC.expired())
		return nullptr;
	return static_cast<pragma::BaseAttachableComponent*>(attC.get())->GetParent();
}

PhysObj *BaseEntity::GetPhysicsObject() const
{
	auto physC = GetPhysicsComponent();
	return physC.valid() ? physC->GetPhysicsObject() : nullptr;
}
PhysObj *BaseEntity::InitializePhysics(PHYSICSTYPE type)
{
	auto *physC = static_cast<pragma::BasePhysicsComponent*>(AddNetworkedComponent("physics").get());
	if(physC == nullptr)
		return false;
	return physC->InitializePhysics(type);
}
void BaseEntity::DestroyPhysicsObject()
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return;
	physC->DestroyPhysicsObject();
}
void BaseEntity::DropToFloor()
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return;
	physC->DropToFloor();
}
std::pair<Vector3,Vector3> BaseEntity::GetCollisionBounds() const
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return {Vector3{},Vector3{}};
	Vector3 min,max;
	physC->GetCollisionBounds(&min,&max);
	return {min,max};
}
void BaseEntity::SetCollisionFilterMask(CollisionMask filterMask)
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return;
	physC->SetCollisionFilterMask(filterMask);
}
void BaseEntity::SetCollisionFilterGroup(CollisionMask filterGroup)
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return;
	physC->SetCollisionFilter(filterGroup);
}
CollisionMask BaseEntity::GetCollisionFilterGroup() const
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return CollisionMask::None;
	return physC->GetCollisionFilter();
}
CollisionMask BaseEntity::GetCollisionFilterMask() const
{
	auto physC = GetPhysicsComponent();
	if(physC.expired())
		return CollisionMask::None;
	return physC->GetCollisionFilterMask();
}

Vector3 BaseEntity::GetForward() const
{
	auto trC = GetTransformComponent();
	return trC.valid() ? trC->GetForward() : uvec::FORWARD;
}
Vector3 BaseEntity::GetUp() const
{
	auto trC = GetTransformComponent();
	return trC.valid() ? trC->GetUp() : uvec::UP;
}
Vector3 BaseEntity::GetRight() const
{
	auto trC = GetTransformComponent();
	return trC.valid() ? trC->GetRight() : uvec::RIGHT;
}

void BaseEntity::Input(const std::string &input,BaseEntity *activator,BaseEntity *caller,const std::string &data)
{
	auto *ioC = static_cast<pragma::BaseIOComponent*>(AddNetworkedComponent("io").get());
	if(ioC == nullptr)
		return;
	ioC->Input(input,activator,caller,data);
}

uint16_t BaseEntity::GetHealth() const
{
	auto healthC = FindComponent("health");
	return healthC.valid() ? static_cast<pragma::BaseHealthComponent&>(*healthC).GetHealth() : 0;
}
uint16_t BaseEntity::GetMaxHealth() const
{
	auto healthC = FindComponent("health");
	return healthC.valid() ? static_cast<pragma::BaseHealthComponent&>(*healthC).GetMaxHealth() : 0;
}
void BaseEntity::SetHealth(uint16_t health)
{
	auto *healthC = static_cast<pragma::BaseHealthComponent*>(AddNetworkedComponent("health").get());
	if(healthC == nullptr)
		return;
	healthC->SetHealth(health);
}
void BaseEntity::SetMaxHealth(uint16_t maxHealth)
{
	auto *healthC = static_cast<pragma::BaseHealthComponent*>(AddNetworkedComponent("health").get());
	if(healthC == nullptr)
		return;
	healthC->SetMaxHealth(maxHealth);
}

void BaseEntity::SetVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<pragma::VelocityComponent*>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->SetVelocity(vel);
}
void BaseEntity::AddVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<pragma::VelocityComponent*>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->AddVelocity(vel);
}
Vector3 BaseEntity::GetVelocity() const
{
	auto velC = GetComponent<pragma::VelocityComponent>();
	return velC.valid() ? velC->GetVelocity() : Vector3{};
}
void BaseEntity::SetAngularVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<pragma::VelocityComponent*>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->SetAngularVelocity(vel);
}
void BaseEntity::AddAngularVelocity(const Vector3 &vel)
{
	auto *velC = static_cast<pragma::VelocityComponent*>(AddNetworkedComponent("velocity").get());
	if(velC == nullptr)
		return;
	velC->AddAngularVelocity(vel);
}
Vector3 BaseEntity::GetAngularVelocity() const
{
	auto velC = GetComponent<pragma::VelocityComponent>();
	return velC.valid() ? velC->GetAngularVelocity() : Vector3{};
}

void BaseEntity::PlayAnimation(int32_t animation,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return;
	animC->PlayAnimation(animation,flags);
}
void BaseEntity::PlayLayeredAnimation(int32_t slot,int32_t animation,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return;
	animC->PlayLayeredAnimation(slot,animation,flags);
}
bool BaseEntity::PlayActivity(Activity activity,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayActivity(activity,flags);
}
bool BaseEntity::PlayLayeredActivity(int32_t slot,Activity activity,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayLayeredActivity(slot,activity,flags);
}
bool BaseEntity::PlayLayeredAnimation(int32_t slot,std::string animation,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayLayeredAnimation(slot,animation,flags);
}
void BaseEntity::StopLayeredAnimation(int slot)
{
	auto animC = GetAnimatedComponent();
	if(animC.expired())
		return;
	animC->StopLayeredAnimation(slot);
}
bool BaseEntity::PlayAnimation(const std::string &animation,pragma::FPlayAnim flags)
{
	auto *animC = static_cast<pragma::BaseAnimatedComponent*>(AddNetworkedComponent("animated").get());
	if(animC == nullptr)
		return false;
	return animC->PlayAnimation(animation,flags);
}
int32_t BaseEntity::GetAnimation() const
{
	auto animC = GetAnimatedComponent();
	return animC.valid() ? animC->GetAnimation() : -1;
}
Activity BaseEntity::GetActivity() const
{
	auto animC = GetAnimatedComponent();
	return animC.valid() ? animC->GetActivity() : Activity::Invalid;
}

void BaseEntity::TakeDamage(DamageInfo &info)
{
	auto *dmgC = static_cast<pragma::DamageableComponent*>(AddNetworkedComponent("damageable").get());
	if(dmgC == nullptr)
		return;
	dmgC->TakeDamage(info);
}
