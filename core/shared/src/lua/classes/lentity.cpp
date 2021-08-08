/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lentity.h"
#include <pragma/math/angle/wvquaternion.h>
#include "luasystem.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/policies/handle_policy.hpp"
#include "pragma/lua/policies/pair_policy.hpp"
#include "pragma/lua/policies/vector_policy.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/libraries/lray.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/audio/alsound.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/raytraces.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_parent_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_generic_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/lua/lentity_type.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include <luabind/copy_policy.hpp>
#include <sharedutils/datastream.h>
#include <pragma/physics/movetypes.h>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

bool Lua::is_entity(const luabind::object &o)
{
	return luabind::object_cast_nothrow<EntityHandle*>(o,static_cast<EntityHandle*>(nullptr));
}

static bool operator==(const BaseEntity &a,const BaseEntity &b)
{
	return &a == &b;
}

static std::ostream &operator<<(std::ostream &stream,const BaseEntity &a)
{
	const_cast<BaseEntity&>(a).print(stream);
	return stream;
}

void Lua::Entity::register_class(luabind::class_<BaseEntity> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid",&IsValid);
	classDef.def("Spawn",&BaseEntity::Spawn);
	classDef.def("Remove",&BaseEntity::Remove);
	classDef.def("GetIndex",&BaseEntity::GetIndex);
	classDef.def("GetLocalIndex",&BaseEntity::GetLocalIndex);
	classDef.def("IsMapEntity",&BaseEntity::IsMapEntity);
	classDef.def("IsCharacter",&BaseEntity::IsCharacter);
	classDef.def("IsPlayer",&BaseEntity::IsPlayer);
	classDef.def("IsWorld",&BaseEntity::IsWorld);
	classDef.def("IsInert",&BaseEntity::IsInert);
	classDef.def("GetClass",&BaseEntity::GetClass);
	//classDef.def("AddCallback",&AddCallback); // Obsolete
	classDef.def("IsScripted",&BaseEntity::IsScripted);
	classDef.def("IsSpawned",&BaseEntity::IsSpawned);
	classDef.def("SetKeyValue",&BaseEntity::SetKeyValue);
	classDef.def("IsNPC",&BaseEntity::IsNPC);
	classDef.def("IsWeapon",&BaseEntity::IsWeapon);
	classDef.def("IsVehicle",&BaseEntity::IsVehicle);
	classDef.def("RemoveSafely",&BaseEntity::RemoveSafely);
	classDef.def("RemoveEntityOnRemoval",static_cast<void(*)(BaseEntity&,BaseEntity&)>(&RemoveEntityOnRemoval));
	classDef.def("RemoveEntityOnRemoval",static_cast<void(*)(BaseEntity&,BaseEntity&,Bool)>(&RemoveEntityOnRemoval));
	classDef.def("GetSpawnFlags",&BaseEntity::GetSpawnFlags);
	classDef.def("GetPose",&BaseEntity::GetPose,luabind::copy_policy<0>{});
	classDef.def("SetPose",static_cast<void(*)(BaseEntity&,const umath::Transform&)>([](BaseEntity &ent,const umath::Transform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("SetPose",static_cast<void(*)(BaseEntity&,const umath::ScaledTransform&)>([](BaseEntity &ent,const umath::ScaledTransform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("GetPos",&BaseEntity::GetPosition,luabind::copy_policy<0>{});
	classDef.def("SetPos",static_cast<void(*)(BaseEntity&,const Vector3&)>([](BaseEntity &ent,const Vector3 &pos) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPosition(pos);
	}));
	classDef.def("GetAngles",static_cast<EulerAngles(*)(BaseEntity&)>([](BaseEntity &ent) -> EulerAngles {
		if(!ent.GetTransformComponent())
			return {};
		return ent.GetTransformComponent()->GetAngles();
	}));
	classDef.def("SetAngles",static_cast<void(*)(BaseEntity&,const EulerAngles&)>([](BaseEntity &ent,const EulerAngles &ang) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang);
	}));
	classDef.def("SetScale",static_cast<void(*)(BaseEntity&,const Vector3&)>([](BaseEntity &ent,const Vector3 &v) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v);
	}));
	classDef.def("GetScale",&BaseEntity::GetScale,luabind::copy_policy<0>{});
	classDef.def("GetRotation",&BaseEntity::GetRotation,luabind::copy_policy<0>{});
	classDef.def("SetRotation",static_cast<void(*)(BaseEntity&,const Quat&)>([](BaseEntity &ent,const Quat &rot) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot);
	}));
	classDef.def("GetCenter",&BaseEntity::GetCenter);

	classDef.def("AddComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(BaseEntity::*)(const std::string&,bool)>(&BaseEntity::AddComponent));
	classDef.def("AddComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(BaseEntity&,const std::string&)>([](BaseEntity &ent,const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> {
		return ent.AddComponent(name);
	}));
	classDef.def("AddComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(BaseEntity::*)(pragma::ComponentId,bool)>(&BaseEntity::AddComponent));
	classDef.def("AddComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(*)(BaseEntity&,pragma::ComponentId)>([](BaseEntity &ent,pragma::ComponentId componentId) {
		return ent.AddComponent(componentId);
	}));
	classDef.def("RemoveComponent",static_cast<void(BaseEntity::*)(pragma::BaseEntityComponent&)>(&BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent",static_cast<void(BaseEntity::*)(const std::string&)>(&BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent",static_cast<void(BaseEntity::*)(pragma::ComponentId)>(&BaseEntity::RemoveComponent));
	classDef.def("ClearComponents",&BaseEntity::ClearComponents);
	classDef.def("HasComponent",static_cast<bool(*)(lua_State*,BaseEntity&,const std::string&)>([](lua_State *l,BaseEntity &ent,const std::string &name) {
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(name,componentId) == false)
			return false;
		return ent.HasComponent(componentId);
	}));
	classDef.def("HasComponent",static_cast<bool(BaseEntity::*)(pragma::ComponentId) const>(&BaseEntity::HasComponent));
	classDef.def("HasComponent",static_cast<bool(*)(BaseEntity&,luabind::object)>([](BaseEntity &ent,luabind::object) {return false;}));
	classDef.def("GetComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(BaseEntity::*)(const std::string&) const>(&BaseEntity::FindComponent));
	classDef.def("GetComponent",static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent>(BaseEntity::*)(pragma::ComponentId) const>(&BaseEntity::FindComponent));
	classDef.def("GetComponent",static_cast<void(*)(BaseEntity&,luabind::object)>([](BaseEntity &ent,luabind::object) {}));
	if(Lua::get_extended_lua_modules_enabled())
	{
		// Shorthand functions
		classDef.def("C",static_cast<luabind::optional<pragma::BaseEntityComponent>(*)(BaseEntity&,const std::string&)>([](BaseEntity &ent,const std::string &name) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(name);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C",static_cast<luabind::optional<pragma::BaseEntityComponent>(*)(BaseEntity&,uint32_t)>([](BaseEntity &ent,uint32_t componentId) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(componentId);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C",static_cast<luabind::optional<pragma::BaseEntityComponent>(*)(BaseEntity&,luabind::object)>([](BaseEntity &ent,luabind::object) -> luabind::optional<pragma::BaseEntityComponent> {return nil;}));
	}
	classDef.def("GetComponents",static_cast<std::vector<util::TSharedHandle<pragma::BaseEntityComponent>>&(BaseEntity::*)()>(&BaseEntity::GetComponents));
	classDef.def("GetTransformComponent",&BaseEntity::GetTransformComponent);
	classDef.def("GetPhysicsComponent",&BaseEntity::GetPhysicsComponent);
	classDef.def("GetGenericComponent",&BaseEntity::GetGenericComponent);
	classDef.def("GetCharacterComponent",&BaseEntity::GetCharacterComponent);
	classDef.def("GetWeaponComponent",&BaseEntity::GetWeaponComponent);
	classDef.def("GetVehicleComponent",&BaseEntity::GetVehicleComponent);
	classDef.def("GetPlayerComponent",&BaseEntity::GetPlayerComponent);
	classDef.def("GetTimeScaleComponent",&BaseEntity::GetTimeScaleComponent);
	classDef.def("GetNameComponent",&BaseEntity::GetNameComponent);
	classDef.def("GetAIComponent",&BaseEntity::GetAIComponent);
	classDef.def("GetModelComponent",&BaseEntity::GetModelComponent);
	classDef.def("GetAnimatedComponent",&BaseEntity::GetAnimatedComponent);
	classDef.def("GetUuid",static_cast<std::string(*)(BaseEntity&)>([](BaseEntity &ent) -> std::string {
		return util::uuid_to_string(ent.GetUuid());
	}));
	classDef.def("SetUuid",static_cast<void(*)(BaseEntity&,const std::string&)>([](BaseEntity &ent,const std::string &uuid) {
		ent.SetUuid(util::uuid_string_to_bytes(uuid));
	}));

	classDef.def("Save",&BaseEntity::Save);
	classDef.def("Load",&BaseEntity::Load);
	classDef.def("Copy",&BaseEntity::Copy);

	classDef.def("GetAirDensity",&GetAirDensity);

	classDef.def("IsStatic",&BaseEntity::IsStatic);
	classDef.def("IsDynamic",&BaseEntity::IsDynamic);

	// Quick-access methods
	classDef.def("CreateSound",&BaseEntity::CreateSound);
	classDef.def("EmitSound",&BaseEntity::EmitSound);
	classDef.def("EmitSound",static_cast<std::shared_ptr<ALSound>(*)(BaseEntity&,const std::string&,ALSoundType,float)>([](BaseEntity &ent,const std::string &sndname,ALSoundType soundType,float gain) {
		return ent.EmitSound(sndname,soundType,gain);
	}));
	classDef.def("EmitSound",static_cast<std::shared_ptr<ALSound>(*)(BaseEntity&,const std::string&,ALSoundType)>([](BaseEntity &ent,const std::string &sndname,ALSoundType soundType) {
		return ent.EmitSound(sndname,soundType);
	}));
	classDef.def("GetName",&BaseEntity::GetName);
	classDef.def("SetName",&BaseEntity::SetName);
	classDef.def("SetModel",static_cast<void(BaseEntity::*)(const std::string&)>(&BaseEntity::SetModel));
	classDef.def("SetModel",static_cast<void(BaseEntity::*)(const std::shared_ptr<Model>&)>(&BaseEntity::SetModel));
	classDef.def("ClearModel",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {
		ent.SetModel(std::shared_ptr<Model>{nullptr});
	}));
	classDef.def("GetModel",&BaseEntity::GetModel);
	classDef.def("GetModelName",&BaseEntity::GetModelName);
	classDef.def("GetAttachmentPose",&BaseEntity::GetAttachmentPose);
	classDef.def("GetSkin",&BaseEntity::GetSkin);
	classDef.def("SetSkin",&BaseEntity::SetSkin);
	classDef.def("GetBodyGroup",&BaseEntity::GetBodyGroup);
	classDef.def("GetBodyGroup",static_cast<std::optional<uint32_t>(*)(BaseEntity&,uint32_t)>([](BaseEntity &ent,uint32_t bgId) -> std::optional<uint32_t> {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return {};
		return mdlC->GetBodyGroup(bgId);
	}));
	classDef.def("SetBodyGroup",&BaseEntity::SetBodyGroup);
	classDef.def("SetBodyGroup",static_cast<void(*)(BaseEntity&,uint32_t,uint32_t)>([](BaseEntity &ent,uint32_t bgId,uint32_t id) {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return;
		mdlC->SetBodyGroup(bgId,id);
	}));
	classDef.def("GetParent",static_cast<luabind::optional<pragma::BaseParentComponent>(*)(BaseEntity&)>([](BaseEntity &ent) -> luabind::optional<pragma::BaseParentComponent> {
		auto *parent = ent.GetParent();
		if(parent == nullptr)
			return nil;
		return parent->GetLuaObject();
	}));
	classDef.def("ClearParent",&BaseEntity::ClearParent);
	classDef.def("SetParent",&BaseEntity::SetParent);
	classDef.def("SetOwner",static_cast<void(*)(BaseEntity&,BaseEntity&)>([](BaseEntity &ent,BaseEntity &hOwner) {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent*>(ent.AddComponent("ownable").get());
		if(ownableC == nullptr)
			return;
		ownableC->SetOwner(hOwner);
	}));
	classDef.def("GetOwner",static_cast<luabind::optional<BaseEntity>(*)(BaseEntity&)>([](BaseEntity &ent) -> luabind::optional<BaseEntity> {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent*>(ent.FindComponent("ownable").get());
		if(ownableC == nullptr)
			return nil;
		auto *owner = ownableC->GetOwner();
		if(owner == nullptr)
			return nil;
		return owner->GetLuaObject();
	}));
	classDef.def("SetEnabled",&Lua::Entity::SetEnabled);
	classDef.def("SetTurnedOn",&Lua::Entity::SetEnabled);
	classDef.def("Enable",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {SetEnabled(ent,true);}));
	classDef.def("TurnOn",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {SetEnabled(ent,true);}));
	classDef.def("Disable",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {SetEnabled(ent,false);}));
	classDef.def("TurnOff",static_cast<void(*)(BaseEntity&)>([](BaseEntity &ent) {SetEnabled(ent,false);}));
	classDef.def("IsEnabled",&Lua::Entity::IsEnabled);
	classDef.def("IsTurnedOn",&Lua::Entity::IsEnabled);
	classDef.def("IsDisabled",&Lua::Entity::IsDisabled);
	classDef.def("IsTurnedOff",&Lua::Entity::IsDisabled);
	classDef.def("SetColor",&Lua::Entity::SetColor);
	classDef.def("GetColor",&Lua::Entity::GetColor);
	classDef.def("GetPhysicsObject",&BaseEntity::GetPhysicsObject);
	classDef.def("InitializePhysics",&BaseEntity::InitializePhysics);
	classDef.def("DestroyPhysicsObject",&BaseEntity::DestroyPhysicsObject);
	classDef.def("DropToFloor",&BaseEntity::DropToFloor);
	classDef.def("GetCollisionBounds",&BaseEntity::GetCollisionBounds);
	classDef.def("SetCollisionFilterMask",&BaseEntity::SetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup",&BaseEntity::SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup",&BaseEntity::GetCollisionFilterGroup);
	classDef.def("GetCollisionFilterMask",&BaseEntity::GetCollisionFilterMask);
	classDef.def("GetForward",&BaseEntity::GetForward);
	classDef.def("GetUp",&BaseEntity::GetUp);
	classDef.def("GetRight",&BaseEntity::GetRight);
	classDef.def("Input",&BaseEntity::Input);
	classDef.def("Input",static_cast<void(*)(BaseEntity&,const std::string&,BaseEntity&,BaseEntity&)>([](BaseEntity &ent,const std::string &input,BaseEntity &activator,BaseEntity &caller) {
		ent.Input(input,&activator,&caller);
	}));
	classDef.def("Input",static_cast<void(*)(BaseEntity&,const std::string&,BaseEntity&)>([](BaseEntity &ent,const std::string &input,BaseEntity &activator) {
		ent.Input(input,&activator);
	}));
	classDef.def("Input",static_cast<void(*)(BaseEntity&,const std::string&)>([](BaseEntity &ent,const std::string &input) {
		ent.Input(input);
	}));
	classDef.def("GetHealth",&BaseEntity::GetHealth);
	classDef.def("GetMaxHealth",&BaseEntity::GetMaxHealth);
	classDef.def("SetHealth",&BaseEntity::SetHealth);
	classDef.def("SetMaxHealth",&BaseEntity::SetMaxHealth);
	classDef.def("SetVelocity",&BaseEntity::SetVelocity);
	classDef.def("AddVelocity",&BaseEntity::AddVelocity);
	classDef.def("SetAngularVelocity",&BaseEntity::SetAngularVelocity);
	classDef.def("AddAngularVelocity",&BaseEntity::AddAngularVelocity);
	classDef.def("GetVelocity",&BaseEntity::GetVelocity);
	classDef.def("GetAngularVelocity",&BaseEntity::GetAngularVelocity);
	/*classDef.def("PlayAnimation",static_cast<void(*)(EntityHandle&,int32_t,uint32_t)>([](BaseEntity &ent,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayAnimation(animation,static_cast<pragma::FPlayAnim>(flags));
	}));
	classDef.def("PlayLayeredAnimation",static_cast<void(*)(EntityHandle&,int32_t,int32_t,uint32_t)>([](BaseEntity &ent,int32_t slot,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayLayeredAnimation(slot,animation,static_cast<pragma::FPlayAnim>(flags));
	}));*/
	classDef.def("PlayActivity",&BaseEntity::PlayActivity);
	classDef.def("PlayActivity",static_cast<bool(*)(BaseEntity&,Activity)>([](BaseEntity &ent,Activity activity) {
		return ent.PlayActivity(activity);
	}));
	classDef.def("PlayLayeredActivity",&BaseEntity::PlayLayeredActivity);
	classDef.def("PlayLayeredAnimation",static_cast<bool(*)(BaseEntity&,int32_t,const std::string&,pragma::FPlayAnim)>([](BaseEntity &ent,int32_t slot,const std::string &anim,pragma::FPlayAnim flags) {
		return ent.PlayLayeredAnimation(slot,anim,flags);
	}));
	classDef.def("StopLayeredAnimation",&BaseEntity::StopLayeredAnimation);
	classDef.def("PlayAnimation",static_cast<void(*)(BaseEntity&,uint32_t,pragma::FPlayAnim)>([](BaseEntity &ent,uint32_t anim,pragma::FPlayAnim flags) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim,flags);
	}));
	classDef.def("PlayAnimation",static_cast<void(*)(BaseEntity&,uint32_t)>([](BaseEntity &ent,uint32_t anim) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim);
	}));
	classDef.def("PlayAnimation",static_cast<bool(BaseEntity::*)(const std::string&,pragma::FPlayAnim)>(&BaseEntity::PlayAnimation));
	classDef.def("PlayAnimation",static_cast<bool(*)(BaseEntity&,const std::string&)>([](BaseEntity &ent,const std::string &anim) {
		return ent.PlayAnimation(anim,pragma::FPlayAnim::Default);
	}));
	classDef.def("GetAnimation",&BaseEntity::GetAnimation);
	classDef.def("GetActivity",&BaseEntity::GetActivity);
	classDef.def("TakeDamage",&BaseEntity::TakeDamage);
	//

	// Enums
	classDef.add_static_constant("TYPE_DEFAULT",umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_LOCAL",umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_SHARED",umath::to_integral(LuaEntityType::Shared));

	classDef.add_static_constant("EVENT_HANDLE_KEY_VALUE",BaseEntity::EVENT_HANDLE_KEY_VALUE);
	classDef.add_static_constant("EVENT_ON_SPAWN",BaseEntity::EVENT_ON_SPAWN);
	classDef.add_static_constant("EVENT_ON_POST_SPAWN",BaseEntity::EVENT_ON_POST_SPAWN);
	classDef.add_static_constant("EVENT_ON_REMOVE",BaseEntity::EVENT_ON_REMOVE);

	classDef.add_static_constant("EVENT_ON_COMPONENT_ADDED",pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	classDef.add_static_constant("EVENT_ON_COMPONENT_REMOVED",pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);

	classDef.def(luabind::const_self ==luabind::const_self);
}

bool Lua::Entity::IsValid(BaseEntity *ent)
{
	return ent != nullptr;
}

void Lua::Entity::RemoveEntityOnRemoval(BaseEntity &ent,BaseEntity &entOther,Bool remove)
{
	ent.RemoveEntityOnRemoval(&entOther,remove);
}
void Lua::Entity::RemoveEntityOnRemoval(BaseEntity &ent,BaseEntity &entOther) {Lua::Entity::RemoveEntityOnRemoval(ent,entOther,true);}
void Lua::Entity::SetEnabled(BaseEntity &ent,bool enabled)
{
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(ent.FindComponent("toggle").get());
	if(toggleC == nullptr && enabled == true)
		return;
	if(toggleC == nullptr)
		toggleC = dynamic_cast<pragma::BaseToggleComponent*>(ent.AddComponent("toggle").get());
	if(toggleC == nullptr)
		return;
	toggleC->SetTurnedOn(enabled);
}

bool Lua::Entity::IsEnabled(BaseEntity &ent)
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(ent.FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return isEnabled;
}

bool Lua::Entity::IsDisabled(BaseEntity &ent)
{
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(ent.FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	return !isEnabled;
}

Color Lua::Entity::GetColor(BaseEntity &ent)
{
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(ent.FindComponent("color").get());
	if(colorC == nullptr)
		return Color::White;
	return colorC->GetColor();
}
void Lua::Entity::SetColor(BaseEntity &ent,const Color &color)
{
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(ent.AddComponent("color").get());
	if(colorC == nullptr)
		return;
	colorC->SetColor(color);
}

float Lua::Entity::GetAirDensity(BaseEntity &ent)
{
	return 1.225f; // Placeholder
}
