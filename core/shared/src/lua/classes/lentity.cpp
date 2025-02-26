/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/lua/classes/lentity.h"
#include <pragma/math/angle/wvquaternion.h>
#include "luasystem.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/libraries/lutil.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/policies/handle_policy.hpp"
#include "pragma/lua/policies/pair_policy.hpp"
#include "pragma/lua/policies/vector_policy.hpp"
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/types/nil_type.hpp"
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
#include "pragma/entities/components/parent_component.hpp"
#include "pragma/entities/components/base_child_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_generic_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/basetoggle.h"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/lentity_type.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include <luabind/copy_policy.hpp>
#include <sharedutils/datastream.h>
#include <pragma/physics/movetypes.h>
#include <udm.hpp>

import panima;

extern DLLNETWORK Engine *engine;

bool Lua::is_entity(const luabind::object &o) { return luabind::object_cast_nothrow<EntityHandle *>(o, static_cast<EntityHandle *>(nullptr)); }

static bool operator==(const BaseEntity &a, const BaseEntity &b) { return &a == &b; }

static std::ostream &operator<<(std::ostream &stream, const BaseEntity &a)
{
	const_cast<BaseEntity &>(a).print(stream);
	return stream;
}

template<typename T>
bool set_member_value(lua_State *l, BaseEntity &ent, const std::string &uri, T value)
{
	auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
	if(!path.has_value())
		return false;
	auto c = ent.FindComponent(path->first);
	if(c.expired())
		return false;
	auto &memberPath = path->second;
	auto *info = c->FindMemberInfo(memberPath.GetString());
	if(!info)
		return false;
	return pragma::lua::set_member_value(l, *c, *info, value);
}

template<typename TValue, auto TSetValue>
bool set_transform_member_value(BaseEntity &ent, const std::string &uri, umath::CoordinateSpace space, const TValue &value)
{
	auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
	if(!path.has_value())
		return false;
	auto c = ent.FindComponent(path->first);
	if(c.expired())
		return false;
	auto &memberPath = path->second;
	auto memberIdx = c->GetMemberIndex(memberPath.GetString());
	if(!memberIdx)
		return false;
	return ((*c).*TSetValue)(*memberIdx, space, value);
}

template<typename TValue, auto TGetValue>
std::optional<TValue> get_transform_member_value(BaseEntity &ent, const std::string &uri, umath::CoordinateSpace space)
{
	auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
	if(!path.has_value())
		return {};
	auto c = ent.FindComponent(path->first);
	if(c.expired())
		return {};
	auto &memberPath = path->second;
	auto memberIdx = c->GetMemberIndex(memberPath.GetString());
	if(!memberIdx)
		return {};
	TValue val;
	if(!((*c).*TGetValue)(*memberIdx, space, val))
		return {};
	return val;
}

void Lua::Entity::register_class(luabind::class_<BaseEntity> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid", &IsValid);
	classDef.def("Spawn", &BaseEntity::Spawn);
	classDef.def("Remove", &BaseEntity::Remove);
	classDef.def("GetIndex", &BaseEntity::GetIndex);
	classDef.def("GetLocalIndex", &BaseEntity::GetLocalIndex);
	classDef.def("IsMapEntity", &BaseEntity::IsMapEntity);
	classDef.def("CreateChild", &BaseEntity::CreateChild);
	classDef.def(
	  "CallOnRemove", +[](lua_State *l, BaseEntity &ent, const Lua::func<void> &function) -> CallbackHandle {
		  return ent.CallOnRemove(FunctionCallback<void>::Create([l, function]() {
			  auto c = Lua::CallFunction(l, [&function](lua_State *l) -> Lua::StatusCode {
				  function.push(l);
				  return Lua::StatusCode::Ok;
			  });
		  }));
	  });
	classDef.def("IsCharacter", &BaseEntity::IsCharacter);
	classDef.def("IsPlayer", &BaseEntity::IsPlayer);
	classDef.def("IsWorld", &BaseEntity::IsWorld);
	classDef.def("IsInert", &BaseEntity::IsInert);
	classDef.def(
	  "GetClass", +[](const BaseEntity &ent) { return std::string {*ent.GetClass()}; });
	//classDef.def("AddCallback",&AddCallback); // Obsolete
	classDef.def("IsScripted", &BaseEntity::IsScripted);
	classDef.def("IsSpawned", &BaseEntity::IsSpawned);
	classDef.def("SetKeyValue", &BaseEntity::SetKeyValue);
	classDef.def("IsNPC", &BaseEntity::IsNPC);
	classDef.def("IsWeapon", &BaseEntity::IsWeapon);
	classDef.def("IsVehicle", &BaseEntity::IsVehicle);
	classDef.def("RemoveSafely", &BaseEntity::RemoveSafely);
	classDef.def("GetUri", static_cast<std::string (BaseEntity::*)() const>(&BaseEntity::GetUri));
	classDef.def("RemoveEntityOnRemoval", static_cast<void (*)(BaseEntity &, BaseEntity &)>(&RemoveEntityOnRemoval));
	classDef.def("RemoveEntityOnRemoval", static_cast<void (*)(BaseEntity &, BaseEntity &, Bool)>(&RemoveEntityOnRemoval));
	classDef.def("GetSpawnFlags", &BaseEntity::GetSpawnFlags);
	classDef.def("GetPose", static_cast<const umath::ScaledTransform &(BaseEntity::*)() const>(&BaseEntity::GetPose), luabind::copy_policy<0> {});
	classDef.def("GetPose", static_cast<umath::ScaledTransform (BaseEntity::*)(pragma::CoordinateSpace) const>(&BaseEntity::GetPose));
	classDef.def("SetPose", static_cast<void (*)(BaseEntity &, const umath::Transform &)>([](BaseEntity &ent, const umath::Transform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("SetPose", static_cast<void (*)(BaseEntity &, const umath::Transform &, pragma::CoordinateSpace)>([](BaseEntity &ent, const umath::Transform &t, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t, space);
	}));
	classDef.def("SetPose", static_cast<void (*)(BaseEntity &, const umath::ScaledTransform &)>([](BaseEntity &ent, const umath::ScaledTransform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("SetPose", static_cast<void (*)(BaseEntity &, const umath::ScaledTransform &, pragma::CoordinateSpace)>([](BaseEntity &ent, const umath::ScaledTransform &t, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t, space);
	}));
	classDef.def("GetPos", static_cast<const Vector3 &(BaseEntity::*)() const>(&BaseEntity::GetPosition), luabind::copy_policy<0> {});
	classDef.def("GetPos", static_cast<Vector3 (BaseEntity::*)(pragma::CoordinateSpace) const>(&BaseEntity::GetPosition));
	classDef.def("SetPos", static_cast<void (*)(BaseEntity &, const Vector3 &)>([](BaseEntity &ent, const Vector3 &pos) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPosition(pos);
	}));
	classDef.def("SetPos", static_cast<void (*)(BaseEntity &, const Vector3 &, pragma::CoordinateSpace)>([](BaseEntity &ent, const Vector3 &pos, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPosition(pos, space);
	}));
	classDef.def("GetAngles", static_cast<EulerAngles (*)(BaseEntity &)>([](BaseEntity &ent) -> EulerAngles {
		if(!ent.GetTransformComponent())
			return {};
		return ent.GetTransformComponent()->GetAngles();
	}));
	classDef.def("SetAngles", static_cast<void (*)(BaseEntity &, const EulerAngles &)>([](BaseEntity &ent, const EulerAngles &ang) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang);
	}));
	classDef.def("SetAngles", static_cast<void (*)(BaseEntity &, const EulerAngles &, pragma::CoordinateSpace)>([](BaseEntity &ent, const EulerAngles &ang, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang, space);
	}));
	classDef.def("SetScale", static_cast<void (*)(BaseEntity &, const Vector3 &)>([](BaseEntity &ent, const Vector3 &v) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v);
	}));
	classDef.def("SetScale", static_cast<void (*)(BaseEntity &, const Vector3 &, pragma::CoordinateSpace)>([](BaseEntity &ent, const Vector3 &v, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v, space);
	}));
	classDef.def("GetScale", static_cast<const Vector3 &(BaseEntity::*)() const>(&BaseEntity::GetScale), luabind::copy_policy<0> {});
	classDef.def("GetScale", static_cast<Vector3 (BaseEntity::*)(pragma::CoordinateSpace) const>(&BaseEntity::GetScale));
	classDef.def("GetRotation", static_cast<const Quat &(BaseEntity::*)() const>(&BaseEntity::GetRotation), luabind::copy_policy<0> {});
	classDef.def("GetRotation", static_cast<Quat (BaseEntity::*)(pragma::CoordinateSpace) const>(&BaseEntity::GetRotation));
	classDef.def("SetRotation", static_cast<void (*)(BaseEntity &, const Quat &)>([](BaseEntity &ent, const Quat &rot) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot);
	}));
	classDef.def("SetRotation", static_cast<void (*)(BaseEntity &, const Quat &, pragma::CoordinateSpace)>([](BaseEntity &ent, const Quat &rot, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot, space);
	}));
	classDef.def("GetCenter", &BaseEntity::GetCenter);

	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (BaseEntity::*)(const std::string &, bool)>(&BaseEntity::AddComponent));
	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(BaseEntity &, const std::string &)>([](BaseEntity &ent, const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return ent.AddComponent(name); }));
	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (BaseEntity::*)(pragma::ComponentId, bool)>(&BaseEntity::AddComponent));
	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(BaseEntity &, pragma::ComponentId)>([](BaseEntity &ent, pragma::ComponentId componentId) { return ent.AddComponent(componentId); }));
	classDef.def(
	  "RemoveComponent", +[](Lua::nil_type) {}); // Don't do anything if component type is nil
	classDef.def("RemoveComponent", static_cast<void (BaseEntity::*)(pragma::BaseEntityComponent &)>(&BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent", static_cast<void (BaseEntity::*)(const std::string &)>(&BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent", static_cast<void (BaseEntity::*)(pragma::ComponentId)>(&BaseEntity::RemoveComponent));
	classDef.def(
	  "ClearComponents", +[](Lua::nil_type) {}); // Don't do anything if component type is nil
	classDef.def("ClearComponents", &BaseEntity::ClearComponents);
	classDef.def(
	  "HasComponent", +[](Lua::nil_type) -> bool { return false; }); // Return false if no component id was specified
	classDef.def("HasComponent", static_cast<bool (*)(lua_State *, BaseEntity &, const std::string &)>([](lua_State *l, BaseEntity &ent, const std::string &name) {
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(name, componentId) == false)
			return false;
		return ent.HasComponent(componentId);
	}));
	classDef.def("HasComponent", static_cast<bool (BaseEntity::*)(pragma::ComponentId) const>(&BaseEntity::HasComponent));
	classDef.def("HasComponent", static_cast<bool (*)(BaseEntity &, luabind::object)>([](BaseEntity &ent, luabind::object) { return false; }));
	classDef.def(
	  "GetComponent", +[](Lua::nil_type) {}); // Return nil if no component id was specified
	classDef.def("GetComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (BaseEntity::*)(const std::string &) const>(&BaseEntity::FindComponent));
	classDef.def("GetComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (BaseEntity::*)(pragma::ComponentId) const>(&BaseEntity::FindComponent));
	classDef.def("GetComponent", static_cast<void (*)(BaseEntity &, luabind::object)>([](BaseEntity &ent, luabind::object) {}));
	if(Lua::get_extended_lua_modules_enabled()) {
		// Shorthand functions
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(BaseEntity &, const std::string &)>([](BaseEntity &ent, const std::string &name) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(name);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(BaseEntity &, uint32_t)>([](BaseEntity &ent, uint32_t componentId) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(componentId);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(BaseEntity &, luabind::object)>([](BaseEntity &ent, luabind::object) -> luabind::optional<pragma::BaseEntityComponent> { return nil; }));
	}
	classDef.def(
	  "GetComponents", +[](lua_State *l, BaseEntity &ent) -> Lua::tb<pragma::BaseEntityComponent> {
		  auto t = luabind::newtable(l);
		  for(uint32_t idx = 1; auto &c : ent.GetComponents()) {
			  if(c.expired())
				  continue;
			  t[idx++] = c->GetLuaObject();
		  }
		  return t;
	  });
	classDef.def("GetTransformComponent", &BaseEntity::GetTransformComponent);
	classDef.def("GetPhysicsComponent", &BaseEntity::GetPhysicsComponent);
	classDef.def("GetGenericComponent", &BaseEntity::GetGenericComponent);
	classDef.def("GetCharacterComponent", &BaseEntity::GetCharacterComponent);
	classDef.def("GetWeaponComponent", &BaseEntity::GetWeaponComponent);
	classDef.def("GetVehicleComponent", &BaseEntity::GetVehicleComponent);
	classDef.def("GetPlayerComponent", &BaseEntity::GetPlayerComponent);
	classDef.def("GetTimeScaleComponent", &BaseEntity::GetTimeScaleComponent);
	classDef.def("GetNameComponent", &BaseEntity::GetNameComponent);
	classDef.def("GetAIComponent", &BaseEntity::GetAIComponent);
	classDef.def("GetModelComponent", &BaseEntity::GetModelComponent);
	classDef.def("GetAnimatedComponent", &BaseEntity::GetAnimatedComponent);
	classDef.def(
	  "FindMemberInfo", +[](lua_State *l, BaseEntity &ent, const std::string &uri) -> Lua::opt<Lua::mult<const pragma::ComponentMemberInfo *, Lua::type<pragma::BaseEntityComponent>>> {
		  auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
		  if(!path.has_value())
			  return Lua::nil;
		  auto c = ent.FindComponent(path->first);
		  if(c.expired())
			  return Lua::nil;
		  auto &memberPath = path->second;
		  return Lua::mult<const pragma::ComponentMemberInfo *, Lua::type<pragma::BaseEntityComponent>> {l, c->FindMemberInfo(memberPath.GetString()), c->GetLuaObject()};
	  });
	classDef.def(
	  "GetMemberValue", +[](lua_State *l, BaseEntity &ent, const std::string &uri) -> std::optional<Lua::udm_type> {
		  auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
		  if(!path.has_value())
			  return {};
		  auto c = ent.FindComponent(path->first);
		  if(c.expired())
			  return {};
		  auto &memberPath = path->second;
		  auto *info = c->FindMemberInfo(memberPath.GetString());
		  if(!info)
			  return {};
		  return pragma::lua::get_member_value(l, *c, *info);
	  });
	classDef.def("SetMemberValue", &set_member_value<Lua::udm_type>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::EntityURef &>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::MultiEntityURef &>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::EntityUComponentMemberRef &>);
	classDef.def("GetTransformMemberPos", &get_transform_member_value<Vector3, &pragma::BaseEntityComponent::GetTransformMemberPos>);
	classDef.def("GetTransformMemberRot", &get_transform_member_value<Quat, &pragma::BaseEntityComponent::GetTransformMemberRot>);
	classDef.def("GetTransformMemberScale", &get_transform_member_value<Vector3, &pragma::BaseEntityComponent::GetTransformMemberScale>);
	classDef.def("GetTransformMemberPose", &get_transform_member_value<umath::ScaledTransform, &pragma::BaseEntityComponent::GetTransformMemberPose>);
	classDef.def("SetTransformMemberPos", &set_transform_member_value<Vector3, &pragma::BaseEntityComponent::SetTransformMemberPos>);
	classDef.def("SetTransformMemberRot", &set_transform_member_value<Quat, &pragma::BaseEntityComponent::SetTransformMemberRot>);
	classDef.def("SetTransformMemberScale", &set_transform_member_value<Vector3, &pragma::BaseEntityComponent::SetTransformMemberScale>);
	classDef.def("SetTransformMemberPose", &set_transform_member_value<umath::ScaledTransform, &pragma::BaseEntityComponent::SetTransformMemberPose>);
	classDef.def("GetUuid", static_cast<std::string (*)(BaseEntity &)>([](BaseEntity &ent) -> std::string { return ::util::uuid_to_string(ent.GetUuid()); }));
	classDef.def("SetUuid", static_cast<void (*)(BaseEntity &, const std::string &)>([](BaseEntity &ent, const std::string &uuid) { ent.SetUuid(::util::uuid_string_to_bytes(uuid)); }));
	classDef.def(
	  "SetUuid", +[](BaseEntity &ent, const Lua::util::Uuid &uuid) { ent.SetUuid(uuid.value); }, luabind::const_ref_policy<2> {});

	classDef.def("Save", &BaseEntity::Save);
	classDef.def("Load", &BaseEntity::Load);
	classDef.def("Copy", &BaseEntity::Copy);

	classDef.def("GetAirDensity", &GetAirDensity);

	classDef.def("IsStatic", &BaseEntity::IsStatic);
	classDef.def("IsDynamic", &BaseEntity::IsDynamic);

	// Quick-access methods
	classDef.def("CreateSound", &BaseEntity::CreateSound);
	classDef.def("EmitSound", &BaseEntity::EmitSound);
	classDef.def("EmitSound", static_cast<std::shared_ptr<ALSound> (*)(BaseEntity &, const std::string &, ALSoundType, float)>([](BaseEntity &ent, const std::string &sndname, ALSoundType soundType, float gain) { return ent.EmitSound(sndname, soundType, gain); }));
	classDef.def("EmitSound", static_cast<std::shared_ptr<ALSound> (*)(BaseEntity &, const std::string &, ALSoundType)>([](BaseEntity &ent, const std::string &sndname, ALSoundType soundType) { return ent.EmitSound(sndname, soundType); }));
	classDef.def("GetName", &BaseEntity::GetName);
	classDef.def("SetName", &BaseEntity::SetName);
	classDef.def("SetModel", static_cast<void (BaseEntity::*)(const std::string &)>(&BaseEntity::SetModel));
	classDef.def("SetModel", static_cast<void (BaseEntity::*)(const std::shared_ptr<Model> &)>(&BaseEntity::SetModel));
	classDef.def("ClearModel", static_cast<void (*)(BaseEntity &)>([](BaseEntity &ent) { ent.SetModel(std::shared_ptr<Model> {nullptr}); }));
	classDef.def("GetModel", &BaseEntity::GetModel);
	classDef.def("GetModelName", &BaseEntity::GetModelName);
	classDef.def("GetAttachmentPose", &BaseEntity::GetAttachmentPose);
	classDef.def("GetSkin", &BaseEntity::GetSkin);
	classDef.def("SetSkin", &BaseEntity::SetSkin);
	classDef.def("GetBodyGroup", &BaseEntity::GetBodyGroup);
	classDef.def("GetBodyGroup", static_cast<std::optional<uint32_t> (*)(BaseEntity &, uint32_t)>([](BaseEntity &ent, uint32_t bgId) -> std::optional<uint32_t> {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return {};
		return mdlC->GetBodyGroup(bgId);
	}));
	classDef.def("SetBodyGroup", &BaseEntity::SetBodyGroup);
	classDef.def("SetBodyGroup", static_cast<void (*)(BaseEntity &, uint32_t, uint32_t)>([](BaseEntity &ent, uint32_t bgId, uint32_t id) {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return;
		mdlC->SetBodyGroup(bgId, id);
	}));
	classDef.def("GetParent", static_cast<luabind::optional<pragma::ParentComponent> (*)(BaseEntity &)>([](BaseEntity &ent) -> luabind::optional<pragma::ParentComponent> {
		auto *parent = ent.GetParent();
		if(parent == nullptr)
			return nil;
		return parent->GetLuaObject();
	}));
	classDef.def("ClearParent", &BaseEntity::ClearParent);
	classDef.def("SetParent", &BaseEntity::SetParent);
	classDef.def("HasParent", &BaseEntity::HasParent);
	classDef.def("HasChildren", &BaseEntity::HasChildren);
	classDef.def("IsChildOf", &BaseEntity::IsChildOf);
	classDef.def("IsAncestorOf", &BaseEntity::IsAncestorOf);
	classDef.def("IsDescendantOf", &BaseEntity::IsDescendantOf);
	classDef.def("IsParentOf", &BaseEntity::IsParentOf);
	classDef.def("SetOwner", static_cast<void (*)(BaseEntity &, BaseEntity &)>([](BaseEntity &ent, BaseEntity &hOwner) {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent *>(ent.AddComponent("ownable").get());
		if(ownableC == nullptr)
			return;
		ownableC->SetOwner(hOwner);
	}));
	classDef.def("GetOwner", static_cast<luabind::optional<BaseEntity> (*)(BaseEntity &)>([](BaseEntity &ent) -> luabind::optional<BaseEntity> {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent *>(ent.FindComponent("ownable").get());
		if(ownableC == nullptr)
			return nil;
		auto *owner = ownableC->GetOwner();
		if(owner == nullptr)
			return nil;
		return owner->GetLuaObject();
	}));
	classDef.def("SetEnabled", &BaseEntity::SetEnabled);
	classDef.def("SetTurnedOn", &BaseEntity::SetEnabled);
	classDef.def("Enable", static_cast<void (*)(BaseEntity &)>([](BaseEntity &ent) { ent.SetEnabled(true); }));
	classDef.def("TurnOn", static_cast<void (*)(BaseEntity &)>([](BaseEntity &ent) { ent.SetEnabled(true); }));
	classDef.def("Disable", static_cast<void (*)(BaseEntity &)>([](BaseEntity &ent) { ent.SetEnabled(false); }));
	classDef.def("TurnOff", static_cast<void (*)(BaseEntity &)>([](BaseEntity &ent) { ent.SetEnabled(false); }));
	classDef.def("IsEnabled", &BaseEntity::IsEnabled);
	classDef.def("IsTurnedOn", &BaseEntity::IsEnabled);
	classDef.def("IsDisabled", &BaseEntity::IsDisabled);
	classDef.def("IsTurnedOff", &BaseEntity::IsDisabled);
	classDef.def("IsRemoved", &BaseEntity::IsRemoved);
	classDef.def("SetColor", &BaseEntity::SetColor);
	classDef.def(
	  "GetColor", +[](BaseEntity &ent) {
		  auto col = ent.GetColor();
		  if(!col.has_value())
			  return Color::White;
		  return *col;
	  });
	classDef.def("GetPhysicsObject", &BaseEntity::GetPhysicsObject);
	classDef.def("InitializePhysics", &BaseEntity::InitializePhysics);
	classDef.def("DestroyPhysicsObject", &BaseEntity::DestroyPhysicsObject);
	classDef.def("DropToFloor", &BaseEntity::DropToFloor);
	classDef.def("GetCollisionBounds", &BaseEntity::GetCollisionBounds);
	classDef.def("SetCollisionFilterMask", &BaseEntity::SetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup", &BaseEntity::SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", &BaseEntity::GetCollisionFilterGroup);
	classDef.def("GetCollisionFilterMask", &BaseEntity::GetCollisionFilterMask);
	classDef.def("GetForward", &BaseEntity::GetForward);
	classDef.def("GetUp", &BaseEntity::GetUp);
	classDef.def("GetRight", &BaseEntity::GetRight);
	classDef.def("Input", &BaseEntity::Input);
	classDef.def("Input", static_cast<void (*)(BaseEntity &, const std::string &, BaseEntity &, BaseEntity &)>([](BaseEntity &ent, const std::string &input, BaseEntity &activator, BaseEntity &caller) { ent.Input(input, &activator, &caller); }));
	classDef.def("Input", static_cast<void (*)(BaseEntity &, const std::string &, BaseEntity &)>([](BaseEntity &ent, const std::string &input, BaseEntity &activator) { ent.Input(input, &activator); }));
	classDef.def("Input", static_cast<void (*)(BaseEntity &, const std::string &)>([](BaseEntity &ent, const std::string &input) { ent.Input(input); }));
	classDef.def("GetHealth", &BaseEntity::GetHealth);
	classDef.def("GetMaxHealth", &BaseEntity::GetMaxHealth);
	classDef.def("SetHealth", &BaseEntity::SetHealth);
	classDef.def("SetMaxHealth", &BaseEntity::SetMaxHealth);
	classDef.def("SetVelocity", &BaseEntity::SetVelocity);
	classDef.def("AddVelocity", &BaseEntity::AddVelocity);
	classDef.def("SetAngularVelocity", &BaseEntity::SetAngularVelocity);
	classDef.def("AddAngularVelocity", &BaseEntity::AddAngularVelocity);
	classDef.def("GetVelocity", &BaseEntity::GetVelocity);
	classDef.def("GetAngularVelocity", &BaseEntity::GetAngularVelocity);
	/*classDef.def("PlayAnimation",static_cast<void(*)(EntityHandle&,int32_t,uint32_t)>([](BaseEntity &ent,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayAnimation(animation,static_cast<pragma::FPlayAnim>(flags));
	}));
	classDef.def("PlayLayeredAnimation",static_cast<void(*)(EntityHandle&,int32_t,int32_t,uint32_t)>([](BaseEntity &ent,int32_t slot,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayLayeredAnimation(slot,animation,static_cast<pragma::FPlayAnim>(flags));
	}));*/
	classDef.def("PlayActivity", &BaseEntity::PlayActivity);
	classDef.def("PlayActivity", static_cast<bool (*)(BaseEntity &, Activity)>([](BaseEntity &ent, Activity activity) { return ent.PlayActivity(activity); }));
	classDef.def("PlayLayeredActivity", &BaseEntity::PlayLayeredActivity);
	classDef.def("PlayLayeredAnimation", static_cast<bool (*)(BaseEntity &, int32_t, const std::string &, pragma::FPlayAnim)>([](BaseEntity &ent, int32_t slot, const std::string &anim, pragma::FPlayAnim flags) { return ent.PlayLayeredAnimation(slot, anim, flags); }));
	classDef.def("StopLayeredAnimation", &BaseEntity::StopLayeredAnimation);
	classDef.def("PlayAnimation", static_cast<void (*)(BaseEntity &, uint32_t, pragma::FPlayAnim)>([](BaseEntity &ent, uint32_t anim, pragma::FPlayAnim flags) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim, flags);
	}));
	classDef.def("PlayAnimation", static_cast<void (*)(BaseEntity &, uint32_t)>([](BaseEntity &ent, uint32_t anim) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim);
	}));
	classDef.def("PlayAnimation", static_cast<bool (BaseEntity::*)(const std::string &, pragma::FPlayAnim)>(&BaseEntity::PlayAnimation));
	classDef.def("PlayAnimation", static_cast<bool (*)(BaseEntity &, const std::string &)>([](BaseEntity &ent, const std::string &anim) { return ent.PlayAnimation(anim, pragma::FPlayAnim::Default); }));
	classDef.def("GetAnimation", &BaseEntity::GetAnimation);
	classDef.def("GetActivity", &BaseEntity::GetActivity);
	classDef.def("TakeDamage", &BaseEntity::TakeDamage);
	classDef.def(
	  "DebugPrintComponentProperties", +[](BaseEntity &ent) {
		  ent.print(Con::cout);
		  Con::cout << Con::endl;
		  auto &components = ent.GetComponents();
		  for(auto &c : components) {
			  if(c.expired())
				  continue;
			  Con::cout << "\t" << c->GetComponentInfo()->name << Con::endl;
			  pragma::ComponentMemberIndex memberIndex = 0;
			  auto *info = c->GetMemberInfo(memberIndex++);
			  if(!info)
				  continue;
			  while(info != nullptr) {
				  Con::cout << "\t\t" << info->GetName() << " (";
				  Con::cout << magic_enum::enum_name(info->type) << "): ";
				  pragma::ents::visit_member(info->type, [info, &c](auto tag) {
					  using T = typename decltype(tag)::type;
					  if constexpr(!udm::is_convertible<T, udm::String>())
						  Con::cout << "Unknown value";
					  else {
						  T value;
						  info->getterFunction(*info, *c, &value);
						  auto str = udm::convert<T, udm::String>(value);
						  Con::cout << str;
					  }
				  });
				  Con::cout << Con::endl;

				  info = c->GetMemberInfo(memberIndex++);
			  }
		  }
	  });
	//

	// Enums
	classDef.add_static_constant("TYPE_DEFAULT", umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_LOCAL", umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_SHARED", umath::to_integral(LuaEntityType::Shared));

	classDef.add_static_constant("EVENT_HANDLE_KEY_VALUE", BaseEntity::EVENT_HANDLE_KEY_VALUE);
	classDef.add_static_constant("EVENT_ON_SPAWN", BaseEntity::EVENT_ON_SPAWN);
	classDef.add_static_constant("EVENT_ON_POST_SPAWN", BaseEntity::EVENT_ON_POST_SPAWN);
	classDef.add_static_constant("EVENT_ON_REMOVE", BaseEntity::EVENT_ON_REMOVE);

	classDef.add_static_constant("EVENT_ON_COMPONENT_ADDED", pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	classDef.add_static_constant("EVENT_ON_COMPONENT_REMOVED", pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);

	classDef.def(luabind::const_self == luabind::const_self);
}

bool Lua::Entity::IsValid(BaseEntity *ent) { return ent != nullptr; }

void Lua::Entity::RemoveEntityOnRemoval(BaseEntity &ent, BaseEntity &entOther, Bool remove) { ent.RemoveEntityOnRemoval(&entOther, remove); }
void Lua::Entity::RemoveEntityOnRemoval(BaseEntity &ent, BaseEntity &entOther) { Lua::Entity::RemoveEntityOnRemoval(ent, entOther, true); }

float Lua::Entity::GetAirDensity(BaseEntity &ent)
{
	return 1.225f; // Placeholder
}
