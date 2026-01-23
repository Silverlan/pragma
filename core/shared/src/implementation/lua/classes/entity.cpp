// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :scripting.lua.classes.entity;

import panima;

namespace Lua {
	DLLNETWORK bool is_entity(const luabind::object &o);
};

bool Lua::is_entity(const luabind::object &o) { return luabind::object_cast_nothrow<EntityHandle *>(o, static_cast<EntityHandle *>(nullptr)); }

namespace pragma::ecs {
	static bool operator==(const BaseEntity &a, const BaseEntity &b) { return &a == &b; }
}

template<typename T>
bool set_member_value(lua::State *l, pragma::ecs::BaseEntity &ent, const std::string &uri, T value)
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
	return pragma::LuaCore::set_member_value(l, *c, *info, value);
}

template<typename TValue, auto TSetValue>
bool set_transform_member_value(pragma::ecs::BaseEntity &ent, const std::string &uri, pragma::math::CoordinateSpace space, const TValue &value)
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
std::optional<TValue> get_transform_member_value(pragma::ecs::BaseEntity &ent, const std::string &uri, pragma::math::CoordinateSpace space)
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

void Lua::Entity::register_class(luabind::class_<pragma::ecs::BaseEntity> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid", &IsValid);
	classDef.def("Spawn", &pragma::ecs::BaseEntity::Spawn);
	classDef.def("Remove", &pragma::ecs::BaseEntity::Remove);
	classDef.def("GetIndex", &pragma::ecs::BaseEntity::GetIndex);
	classDef.def("GetLocalIndex", &pragma::ecs::BaseEntity::GetLocalIndex);
	classDef.def("IsMapEntity", &pragma::ecs::BaseEntity::IsMapEntity);
	classDef.def("CreateChild", &pragma::ecs::BaseEntity::CreateChild);
	classDef.def(
	  "CallOnRemove", +[](lua::State *l, pragma::ecs::BaseEntity &ent, const func<void> &function) -> CallbackHandle {
		  return ent.CallOnRemove(FunctionCallback<void>::Create([l, function]() {
			  auto c = CallFunction(l, [&function](lua::State *l) -> StatusCode {
				  function.push(l);
				  return StatusCode::Ok;
			  });
		  }));
	  });
	classDef.def("IsCharacter", &pragma::ecs::BaseEntity::IsCharacter);
	classDef.def("IsPlayer", &pragma::ecs::BaseEntity::IsPlayer);
	classDef.def("IsWorld", &pragma::ecs::BaseEntity::IsWorld);
	classDef.def("IsInert", &pragma::ecs::BaseEntity::IsInert);
	classDef.def("GetClass", +[](const pragma::ecs::BaseEntity &ent) { return std::string {*ent.GetClass()}; });
	//classDef.def("AddCallback",&AddCallback); // Obsolete
	classDef.def("IsScripted", &pragma::ecs::BaseEntity::IsScripted);
	classDef.def("IsSpawned", &pragma::ecs::BaseEntity::IsSpawned);
	classDef.def("SetKeyValue", &pragma::ecs::BaseEntity::SetKeyValue);
	classDef.def("IsNPC", &pragma::ecs::BaseEntity::IsNPC);
	classDef.def("IsWeapon", &pragma::ecs::BaseEntity::IsWeapon);
	classDef.def("IsVehicle", &pragma::ecs::BaseEntity::IsVehicle);
	classDef.def("RemoveSafely", &pragma::ecs::BaseEntity::RemoveSafely);
	classDef.def("GetUri", static_cast<std::string (pragma::ecs::BaseEntity::*)() const>(&pragma::ecs::BaseEntity::GetUri));
	classDef.def("RemoveEntityOnRemoval", static_cast<void (*)(pragma::ecs::BaseEntity &, pragma::ecs::BaseEntity &)>(&RemoveEntityOnRemoval));
	classDef.def("RemoveEntityOnRemoval", static_cast<void (*)(pragma::ecs::BaseEntity &, pragma::ecs::BaseEntity &, Bool)>(&RemoveEntityOnRemoval));
	classDef.def("GetSpawnFlags", &pragma::ecs::BaseEntity::GetSpawnFlags);
	classDef.def("GetPose", static_cast<const pragma::math::ScaledTransform &(pragma::ecs::BaseEntity::*)() const>(&pragma::ecs::BaseEntity::GetPose), luabind::copy_policy<0> {});
	classDef.def("GetPose", static_cast<pragma::math::ScaledTransform (pragma::ecs::BaseEntity::*)(pragma::CoordinateSpace) const>(&pragma::ecs::BaseEntity::GetPose));
	classDef.def("SetPose", static_cast<void (*)(pragma::ecs::BaseEntity &, const pragma::math::Transform &)>([](pragma::ecs::BaseEntity &ent, const pragma::math::Transform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("SetPose", static_cast<void (*)(pragma::ecs::BaseEntity &, const pragma::math::Transform &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const pragma::math::Transform &t, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t, space);
	}));
	classDef.def("SetPose", static_cast<void (*)(pragma::ecs::BaseEntity &, const pragma::math::ScaledTransform &)>([](pragma::ecs::BaseEntity &ent, const pragma::math::ScaledTransform &t) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t);
	}));
	classDef.def("SetPose", static_cast<void (*)(pragma::ecs::BaseEntity &, const pragma::math::ScaledTransform &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const pragma::math::ScaledTransform &t, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPose(t, space);
	}));
	classDef.def("GetPos", static_cast<const Vector3 &(pragma::ecs::BaseEntity::*)() const>(&pragma::ecs::BaseEntity::GetPosition), luabind::copy_policy<0> {});
	classDef.def("GetPos", static_cast<Vector3 (pragma::ecs::BaseEntity::*)(pragma::CoordinateSpace) const>(&pragma::ecs::BaseEntity::GetPosition));
	classDef.def("SetPos", static_cast<void (*)(pragma::ecs::BaseEntity &, const Vector3 &)>([](pragma::ecs::BaseEntity &ent, const Vector3 &pos) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPosition(pos);
	}));
	classDef.def("SetPos", static_cast<void (*)(pragma::ecs::BaseEntity &, const Vector3 &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const Vector3 &pos, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		ent.SetPosition(pos, space);
	}));
	classDef.def("GetAngles", static_cast<EulerAngles (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) -> EulerAngles {
		if(!ent.GetTransformComponent())
			return {};
		return ent.GetTransformComponent()->GetAngles();
	}));
	classDef.def("SetAngles", static_cast<void (*)(pragma::ecs::BaseEntity &, const EulerAngles &)>([](pragma::ecs::BaseEntity &ent, const EulerAngles &ang) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang);
	}));
	classDef.def("SetAngles", static_cast<void (*)(pragma::ecs::BaseEntity &, const EulerAngles &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const EulerAngles &ang, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang, space);
	}));
	classDef.def("SetScale", static_cast<void (*)(pragma::ecs::BaseEntity &, const Vector3 &)>([](pragma::ecs::BaseEntity &ent, const Vector3 &v) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v);
	}));
	classDef.def("SetScale", static_cast<void (*)(pragma::ecs::BaseEntity &, const Vector3 &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const Vector3 &v, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v, space);
	}));
	classDef.def("GetScale", static_cast<const Vector3 &(pragma::ecs::BaseEntity::*)() const>(&pragma::ecs::BaseEntity::GetScale), luabind::copy_policy<0> {});
	classDef.def("GetScale", static_cast<Vector3 (pragma::ecs::BaseEntity::*)(pragma::CoordinateSpace) const>(&pragma::ecs::BaseEntity::GetScale));
	classDef.def("GetRotation", static_cast<const Quat &(pragma::ecs::BaseEntity::*)() const>(&pragma::ecs::BaseEntity::GetRotation), luabind::copy_policy<0> {});
	classDef.def("GetRotation", static_cast<Quat (pragma::ecs::BaseEntity::*)(pragma::CoordinateSpace) const>(&pragma::ecs::BaseEntity::GetRotation));
	classDef.def("SetRotation", static_cast<void (*)(pragma::ecs::BaseEntity &, const Quat &)>([](pragma::ecs::BaseEntity &ent, const Quat &rot) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot);
	}));
	classDef.def("SetRotation", static_cast<void (*)(pragma::ecs::BaseEntity &, const Quat &, pragma::CoordinateSpace)>([](pragma::ecs::BaseEntity &ent, const Quat &rot, pragma::CoordinateSpace space) {
		auto *trComponent = static_cast<pragma::BaseTransformComponent *>(ent.AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot, space);
	}));
	classDef.def("GetCenter", &pragma::ecs::BaseEntity::GetCenter);

	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (pragma::ecs::BaseEntity::*)(const std::string &, bool)>(&pragma::ecs::BaseEntity::AddComponent));
	classDef.def("AddComponent",
	  static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(pragma::ecs::BaseEntity &, const std::string &)>([](pragma::ecs::BaseEntity &ent, const std::string &name) -> pragma::ComponentHandle<pragma::BaseEntityComponent> { return ent.AddComponent(name); }));
	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (pragma::ecs::BaseEntity::*)(pragma::ComponentId, bool)>(&pragma::ecs::BaseEntity::AddComponent));
	classDef.def("AddComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (*)(pragma::ecs::BaseEntity &, pragma::ComponentId)>([](pragma::ecs::BaseEntity &ent, pragma::ComponentId componentId) { return ent.AddComponent(componentId); }));
	classDef.def("RemoveComponent", +[](nil_type) {}); // Don't do anything if component type is nil
	classDef.def("RemoveComponent", static_cast<void (pragma::ecs::BaseEntity::*)(pragma::BaseEntityComponent &)>(&pragma::ecs::BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent", static_cast<void (pragma::ecs::BaseEntity::*)(const std::string &)>(&pragma::ecs::BaseEntity::RemoveComponent));
	classDef.def("RemoveComponent", static_cast<void (pragma::ecs::BaseEntity::*)(pragma::ComponentId)>(&pragma::ecs::BaseEntity::RemoveComponent));
	classDef.def("ClearComponents", +[](nil_type) {}); // Don't do anything if component type is nil
	classDef.def("ClearComponents", &pragma::ecs::BaseEntity::ClearComponents);
	classDef.def("HasComponent", +[](nil_type) -> bool { return false; }); // Return false if no component id was specified
	classDef.def("HasComponent", static_cast<bool (*)(lua::State *, pragma::ecs::BaseEntity &, const std::string &)>([](lua::State *l, pragma::ecs::BaseEntity &ent, const std::string &name) {
		auto *nw = pragma::Engine::Get()->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(name, componentId) == false)
			return false;
		return ent.HasComponent(componentId);
	}));
	classDef.def("HasComponent", static_cast<bool (pragma::ecs::BaseEntity::*)(pragma::ComponentId) const>(&pragma::ecs::BaseEntity::HasComponent));
	classDef.def("HasComponent", static_cast<bool (*)(pragma::ecs::BaseEntity &, luabind::object)>([](pragma::ecs::BaseEntity &ent, luabind::object) { return false; }));
	classDef.def("GetComponent", +[](nil_type) {}); // Return nil if no component id was specified
	classDef.def("GetComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (pragma::ecs::BaseEntity::*)(const std::string &) const>(&pragma::ecs::BaseEntity::FindComponent));
	classDef.def("GetComponent", static_cast<pragma::ComponentHandle<pragma::BaseEntityComponent> (pragma::ecs::BaseEntity::*)(pragma::ComponentId) const>(&pragma::ecs::BaseEntity::FindComponent));
	classDef.def("GetComponent", static_cast<void (*)(pragma::ecs::BaseEntity &, luabind::object)>([](pragma::ecs::BaseEntity &ent, luabind::object) {}));
	if(get_extended_lua_modules_enabled()) {
		// Shorthand functions
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(pragma::ecs::BaseEntity &, const std::string &)>([](pragma::ecs::BaseEntity &ent, const std::string &name) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(name);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(pragma::ecs::BaseEntity &, uint32_t)>([](pragma::ecs::BaseEntity &ent, uint32_t componentId) -> luabind::optional<pragma::BaseEntityComponent> {
			auto pComponent = ent.FindComponent(componentId);
			if(pComponent.expired())
				return nil;
			return pComponent->GetLuaObject();
		}));
		classDef.def("C", static_cast<luabind::optional<pragma::BaseEntityComponent> (*)(pragma::ecs::BaseEntity &, luabind::object)>([](pragma::ecs::BaseEntity &ent, luabind::object) -> luabind::optional<pragma::BaseEntityComponent> { return nil; }));
	}
	classDef.def(
	  "GetComponents", +[](lua::State *l, pragma::ecs::BaseEntity &ent) -> tb<pragma::BaseEntityComponent> {
		  auto t = luabind::newtable(l);
		  for(uint32_t idx = 1; auto &c : ent.GetComponents()) {
			  if(c.expired())
				  continue;
			  t[idx++] = c->GetLuaObject();
		  }
		  return t;
	  });
	classDef.def("GetTransformComponent", &pragma::ecs::BaseEntity::GetTransformComponent);
	classDef.def("GetPhysicsComponent", &pragma::ecs::BaseEntity::GetPhysicsComponent);
	classDef.def("GetGenericComponent", &pragma::ecs::BaseEntity::GetGenericComponent);
	classDef.def("GetCharacterComponent", &pragma::ecs::BaseEntity::GetCharacterComponent);
	classDef.def("GetWeaponComponent", &pragma::ecs::BaseEntity::GetWeaponComponent);
	classDef.def("GetVehicleComponent", &pragma::ecs::BaseEntity::GetVehicleComponent);
	classDef.def("GetPlayerComponent", &pragma::ecs::BaseEntity::GetPlayerComponent);
	classDef.def("GetTimeScaleComponent", &pragma::ecs::BaseEntity::GetTimeScaleComponent);
	classDef.def("GetNameComponent", &pragma::ecs::BaseEntity::GetNameComponent);
	classDef.def("GetAIComponent", &pragma::ecs::BaseEntity::GetAIComponent);
	classDef.def("GetModelComponent", &pragma::ecs::BaseEntity::GetModelComponent);
	classDef.def("GetAnimatedComponent", &pragma::ecs::BaseEntity::GetAnimatedComponent);
	classDef.def(
	  "FindMemberInfo", +[](lua::State *l, pragma::ecs::BaseEntity &ent, const std::string &uri) -> opt<mult<const pragma::ComponentMemberInfo *, type<pragma::BaseEntityComponent>>> {
		  auto path = pragma::PanimaComponent::ParseComponentChannelPath(panima::ChannelPath {uri});
		  if(!path.has_value())
			  return nil;
		  auto c = ent.FindComponent(path->first);
		  if(c.expired())
			  return nil;
		  auto &memberPath = path->second;
		  return Lua::mult<const pragma::ComponentMemberInfo *, type<pragma::BaseEntityComponent>> {l, c->FindMemberInfo(memberPath.GetString()), c->GetLuaObject()};
	  });
	classDef.def(
	  "GetMemberValue", +[](lua::State *l, pragma::ecs::BaseEntity &ent, const std::string &uri) -> std::optional<udm_type> {
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
		  return pragma::LuaCore::get_member_value(l, *c, *info);
	  });
	classDef.def("SetMemberValue", &set_member_value<udm_type>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::EntityURef &>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::MultiEntityURef &>);
	classDef.def("SetMemberValue", &set_member_value<const pragma::EntityUComponentMemberRef &>);
	classDef.def("GetTransformMemberPos", &get_transform_member_value<Vector3, &pragma::BaseEntityComponent::GetTransformMemberPos>);
	classDef.def("GetTransformMemberRot", &get_transform_member_value<Quat, &pragma::BaseEntityComponent::GetTransformMemberRot>);
	classDef.def("GetTransformMemberScale", &get_transform_member_value<Vector3, &pragma::BaseEntityComponent::GetTransformMemberScale>);
	classDef.def("GetTransformMemberPose", &get_transform_member_value<pragma::math::ScaledTransform, &pragma::BaseEntityComponent::GetTransformMemberPose>);
	classDef.def("SetTransformMemberPos", &set_transform_member_value<Vector3, &pragma::BaseEntityComponent::SetTransformMemberPos>);
	classDef.def("SetTransformMemberRot", &set_transform_member_value<Quat, &pragma::BaseEntityComponent::SetTransformMemberRot>);
	classDef.def("SetTransformMemberScale", &set_transform_member_value<Vector3, &pragma::BaseEntityComponent::SetTransformMemberScale>);
	classDef.def("SetTransformMemberPose", &set_transform_member_value<pragma::math::ScaledTransform, &pragma::BaseEntityComponent::SetTransformMemberPose>);
	classDef.def("GetUuid", static_cast<std::string (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) -> std::string { return pragma::util::uuid_to_string(ent.GetUuid()); }));
	classDef.def("SetUuid", static_cast<void (*)(pragma::ecs::BaseEntity &, const std::string &)>([](pragma::ecs::BaseEntity &ent, const std::string &uuid) { ent.SetUuid(pragma::util::uuid_string_to_bytes(uuid)); }));
	classDef.def("SetUuid", +[](pragma::ecs::BaseEntity &ent, const util::Uuid &uuid) { ent.SetUuid(uuid.value); }, luabind::const_ref_policy<2> {});

	classDef.def("Save", &pragma::ecs::BaseEntity::Save);
	classDef.def("Load", &pragma::ecs::BaseEntity::Load);
	classDef.def("Copy", &pragma::ecs::BaseEntity::Copy);

	classDef.def("GetAirDensity", &GetAirDensity);

	classDef.def("IsStatic", &pragma::ecs::BaseEntity::IsStatic);
	classDef.def("IsDynamic", &pragma::ecs::BaseEntity::IsDynamic);

	// Quick-access methods
	classDef.def("CreateSound", &pragma::ecs::BaseEntity::CreateSound);
	classDef.def("EmitSound", &pragma::ecs::BaseEntity::EmitSound);
	classDef.def("EmitSound", static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(pragma::ecs::BaseEntity &, const std::string &, pragma::audio::ALSoundType, float)>([](pragma::ecs::BaseEntity &ent, const std::string &sndname, pragma::audio::ALSoundType soundType, float gain) {
		return ent.EmitSound(sndname, soundType, gain);
	}));
	classDef.def("EmitSound",
	  static_cast<std::shared_ptr<pragma::audio::ALSound> (*)(pragma::ecs::BaseEntity &, const std::string &, pragma::audio::ALSoundType)>([](pragma::ecs::BaseEntity &ent, const std::string &sndname, pragma::audio::ALSoundType soundType) { return ent.EmitSound(sndname, soundType); }));
	classDef.def("GetName", &pragma::ecs::BaseEntity::GetName);
	classDef.def("SetName", &pragma::ecs::BaseEntity::SetName);
	classDef.def("SetModel", static_cast<void (pragma::ecs::BaseEntity::*)(const std::string &)>(&pragma::ecs::BaseEntity::SetModel));
	classDef.def("SetModel", static_cast<void (pragma::ecs::BaseEntity::*)(const std::shared_ptr<pragma::asset::Model> &)>(&pragma::ecs::BaseEntity::SetModel));
	classDef.def("ClearModel", static_cast<void (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) { ent.SetModel(std::shared_ptr<pragma::asset::Model> {nullptr}); }));
	classDef.def("GetModel", &pragma::ecs::BaseEntity::GetModel);
	classDef.def("GetModelName", &pragma::ecs::BaseEntity::GetModelName);
	classDef.def("GetAttachmentPose", &pragma::ecs::BaseEntity::GetAttachmentPose);
	classDef.def("GetSkin", &pragma::ecs::BaseEntity::GetSkin);
	classDef.def("SetSkin", &pragma::ecs::BaseEntity::SetSkin);
	classDef.def("GetBodyGroup", &pragma::ecs::BaseEntity::GetBodyGroup);
	classDef.def("GetBodyGroup", static_cast<std::optional<uint32_t> (*)(pragma::ecs::BaseEntity &, uint32_t)>([](pragma::ecs::BaseEntity &ent, uint32_t bgId) -> std::optional<uint32_t> {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return {};
		return mdlC->GetBodyGroup(bgId);
	}));
	classDef.def("SetBodyGroup", &pragma::ecs::BaseEntity::SetBodyGroup);
	classDef.def("SetBodyGroup", static_cast<void (*)(pragma::ecs::BaseEntity &, uint32_t, uint32_t)>([](pragma::ecs::BaseEntity &ent, uint32_t bgId, uint32_t id) {
		auto mdlC = ent.GetModelComponent();
		if(!mdlC)
			return;
		mdlC->SetBodyGroup(bgId, id);
	}));
	classDef.def("GetParent", static_cast<luabind::optional<pragma::ParentComponent> (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) -> luabind::optional<pragma::ParentComponent> {
		auto *parent = ent.GetParent();
		if(parent == nullptr)
			return nil;
		return parent->GetLuaObject();
	}));
	classDef.def("ClearParent", &pragma::ecs::BaseEntity::ClearParent);
	classDef.def("SetParent", &pragma::ecs::BaseEntity::SetParent);
	classDef.def("HasParent", &pragma::ecs::BaseEntity::HasParent);
	classDef.def("HasChildren", &pragma::ecs::BaseEntity::HasChildren);
	classDef.def("IsChildOf", &pragma::ecs::BaseEntity::IsChildOf);
	classDef.def("IsAncestorOf", &pragma::ecs::BaseEntity::IsAncestorOf);
	classDef.def("IsDescendantOf", &pragma::ecs::BaseEntity::IsDescendantOf);
	classDef.def("IsParentOf", &pragma::ecs::BaseEntity::IsParentOf);
	classDef.def("SetOwner", static_cast<void (*)(pragma::ecs::BaseEntity &, pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent, pragma::ecs::BaseEntity &hOwner) {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent *>(ent.AddComponent("ownable").get());
		if(ownableC == nullptr)
			return;
		ownableC->SetOwner(hOwner);
	}));
	classDef.def("GetOwner", static_cast<luabind::optional<pragma::ecs::BaseEntity> (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) -> luabind::optional<pragma::ecs::BaseEntity> {
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent *>(ent.FindComponent("ownable").get());
		if(ownableC == nullptr)
			return nil;
		auto *owner = ownableC->GetOwner();
		if(owner == nullptr)
			return nil;
		return owner->GetLuaObject();
	}));
	classDef.def("SetEnabled", &pragma::ecs::BaseEntity::SetEnabled);
	classDef.def("SetTurnedOn", &pragma::ecs::BaseEntity::SetEnabled);
	classDef.def("Enable", static_cast<void (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) { ent.SetEnabled(true); }));
	classDef.def("TurnOn", static_cast<void (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) { ent.SetEnabled(true); }));
	classDef.def("Disable", static_cast<void (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) { ent.SetEnabled(false); }));
	classDef.def("TurnOff", static_cast<void (*)(pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent) { ent.SetEnabled(false); }));
	classDef.def("IsEnabled", &pragma::ecs::BaseEntity::IsEnabled);
	classDef.def("IsTurnedOn", &pragma::ecs::BaseEntity::IsEnabled);
	classDef.def("IsDisabled", &pragma::ecs::BaseEntity::IsDisabled);
	classDef.def("IsTurnedOff", &pragma::ecs::BaseEntity::IsDisabled);
	classDef.def("IsRemoved", &pragma::ecs::BaseEntity::IsRemoved);
	classDef.def("SetColor", &pragma::ecs::BaseEntity::SetColor);
	classDef.def(
	  "GetColor", +[](pragma::ecs::BaseEntity &ent) {
		  auto col = ent.GetColor();
		  if(!col.has_value())
			  return colors::White;
		  return *col;
	  });
	classDef.def("GetPhysicsObject", &pragma::ecs::BaseEntity::GetPhysicsObject);
	classDef.def("InitializePhysics", &pragma::ecs::BaseEntity::InitializePhysics);
	classDef.def("DestroyPhysicsObject", &pragma::ecs::BaseEntity::DestroyPhysicsObject);
	classDef.def("DropToFloor", &pragma::ecs::BaseEntity::DropToFloor);
	classDef.def("GetCollisionBounds", &pragma::ecs::BaseEntity::GetCollisionBounds);
	classDef.def("SetCollisionFilterMask", &pragma::ecs::BaseEntity::SetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup", &pragma::ecs::BaseEntity::SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", &pragma::ecs::BaseEntity::GetCollisionFilterGroup);
	classDef.def("GetCollisionFilterMask", &pragma::ecs::BaseEntity::GetCollisionFilterMask);
	classDef.def("GetForward", &pragma::ecs::BaseEntity::GetForward);
	classDef.def("GetUp", &pragma::ecs::BaseEntity::GetUp);
	classDef.def("GetRight", &pragma::ecs::BaseEntity::GetRight);
	classDef.def("Input", &pragma::ecs::BaseEntity::Input);
	classDef.def("Input",
	  static_cast<void (*)(pragma::ecs::BaseEntity &, const std::string &, pragma::ecs::BaseEntity &, pragma::ecs::BaseEntity &)>(
	    [](pragma::ecs::BaseEntity &ent, const std::string &input, pragma::ecs::BaseEntity &activator, pragma::ecs::BaseEntity &caller) { ent.Input(input, &activator, &caller); }));
	classDef.def("Input", static_cast<void (*)(pragma::ecs::BaseEntity &, const std::string &, pragma::ecs::BaseEntity &)>([](pragma::ecs::BaseEntity &ent, const std::string &input, pragma::ecs::BaseEntity &activator) { ent.Input(input, &activator); }));
	classDef.def("Input", static_cast<void (*)(pragma::ecs::BaseEntity &, const std::string &)>([](pragma::ecs::BaseEntity &ent, const std::string &input) { ent.Input(input); }));
	classDef.def("GetHealth", &pragma::ecs::BaseEntity::GetHealth);
	classDef.def("GetMaxHealth", &pragma::ecs::BaseEntity::GetMaxHealth);
	classDef.def("SetHealth", &pragma::ecs::BaseEntity::SetHealth);
	classDef.def("SetMaxHealth", &pragma::ecs::BaseEntity::SetMaxHealth);
	classDef.def("SetVelocity", &pragma::ecs::BaseEntity::SetVelocity);
	classDef.def("AddVelocity", &pragma::ecs::BaseEntity::AddVelocity);
	classDef.def("SetAngularVelocity", &pragma::ecs::BaseEntity::SetAngularVelocity);
	classDef.def("AddAngularVelocity", &pragma::ecs::BaseEntity::AddAngularVelocity);
	classDef.def("GetVelocity", &pragma::ecs::BaseEntity::GetVelocity);
	classDef.def("GetAngularVelocity", &pragma::ecs::BaseEntity::GetAngularVelocity);
	/*classDef.def("PlayAnimation",static_cast<void(*)(EntityHandle&,int32_t,uint32_t)>([](BaseEntity &ent,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayAnimation(animation,static_cast<pragma::FPlayAnim>(flags));
	}));
	classDef.def("PlayLayeredAnimation",static_cast<void(*)(EntityHandle&,int32_t,int32_t,uint32_t)>([](BaseEntity &ent,int32_t slot,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		ent.PlayLayeredAnimation(slot,animation,static_cast<pragma::FPlayAnim>(flags));
	}));*/
	classDef.def("PlayActivity", &pragma::ecs::BaseEntity::PlayActivity);
	classDef.def("PlayActivity", static_cast<bool (*)(pragma::ecs::BaseEntity &, pragma::Activity)>([](pragma::ecs::BaseEntity &ent, pragma::Activity activity) { return ent.PlayActivity(activity); }));
	classDef.def("PlayLayeredActivity", &pragma::ecs::BaseEntity::PlayLayeredActivity);
	classDef.def("PlayLayeredAnimation",
	  static_cast<bool (*)(pragma::ecs::BaseEntity &, int32_t, const std::string &, pragma::FPlayAnim)>([](pragma::ecs::BaseEntity &ent, int32_t slot, const std::string &anim, pragma::FPlayAnim flags) { return ent.PlayLayeredAnimation(slot, anim, flags); }));
	classDef.def("StopLayeredAnimation", &pragma::ecs::BaseEntity::StopLayeredAnimation);
	classDef.def("PlayAnimation", static_cast<void (*)(pragma::ecs::BaseEntity &, uint32_t, pragma::FPlayAnim)>([](pragma::ecs::BaseEntity &ent, uint32_t anim, pragma::FPlayAnim flags) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim, flags);
	}));
	classDef.def("PlayAnimation", static_cast<void (*)(pragma::ecs::BaseEntity &, uint32_t)>([](pragma::ecs::BaseEntity &ent, uint32_t anim) {
		auto animC = ent.GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim);
	}));
	classDef.def("PlayAnimation", static_cast<bool (pragma::ecs::BaseEntity::*)(const std::string &, pragma::FPlayAnim)>(&pragma::ecs::BaseEntity::PlayAnimation));
	classDef.def("PlayAnimation", static_cast<bool (*)(pragma::ecs::BaseEntity &, const std::string &)>([](pragma::ecs::BaseEntity &ent, const std::string &anim) { return ent.PlayAnimation(anim, pragma::FPlayAnim::Default); }));
	classDef.def("GetAnimation", &pragma::ecs::BaseEntity::GetAnimation);
	classDef.def("GetActivity", &pragma::ecs::BaseEntity::GetActivity);
	classDef.def("TakeDamage", &pragma::ecs::BaseEntity::TakeDamage);
	classDef.def(
	  "DebugPrintComponentProperties", +[](pragma::ecs::BaseEntity &ent) {
		  ent.print(Con::COUT);
		  Con::COUT << Con::endl;
		  auto &components = ent.GetComponents();
		  for(auto &c : components) {
			  if(c.expired())
				  continue;
			  Con::COUT << "\t" << c->GetComponentInfo()->name << Con::endl;
			  pragma::ComponentMemberIndex memberIndex = 0;
			  auto *info = c->GetMemberInfo(memberIndex++);
			  if(!info)
				  continue;
			  while(info != nullptr) {
				  Con::COUT << "\t\t" << info->GetName() << " (";
				  Con::COUT << magic_enum::enum_name(info->type) << "): ";
				  pragma::ents::visit_member(info->type, [info, &c](auto tag) {
					  using T = typename decltype(tag)::type;
					  if constexpr(!::udm::is_convertible<T, ::udm::String>())
						  Con::COUT << "Unknown value";
					  else {
						  T value;
						  info->getterFunction(*info, *c, &value);
						  auto str = ::udm::convert<T, ::udm::String>(value);
						  Con::COUT << str;
					  }
				  });
				  Con::COUT << Con::endl;

				  info = c->GetMemberInfo(memberIndex++);
			  }
		  }
	  });
	//

	// Enums
	classDef.add_static_constant("TYPE_DEFAULT", pragma::math::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_LOCAL", pragma::math::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_SHARED", pragma::math::to_integral(LuaEntityType::Shared));

	classDef.add_static_constant("EVENT_HANDLE_KEY_VALUE", pragma::ecs::baseEntity::EVENT_HANDLE_KEY_VALUE);
	classDef.add_static_constant("EVENT_ON_SPAWN", pragma::ecs::baseEntity::EVENT_ON_SPAWN);
	classDef.add_static_constant("EVENT_ON_POST_SPAWN", pragma::ecs::baseEntity::EVENT_ON_POST_SPAWN);
	classDef.add_static_constant("EVENT_ON_REMOVE", pragma::ecs::baseEntity::EVENT_ON_REMOVE);

	classDef.add_static_constant("EVENT_ON_COMPONENT_ADDED", pragma::baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	classDef.add_static_constant("EVENT_ON_COMPONENT_REMOVED", pragma::baseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);

	classDef.def(luabind::const_self == luabind::const_self);
}

bool Lua::Entity::IsValid(pragma::ecs::BaseEntity *ent) { return ent != nullptr; }

void Lua::Entity::RemoveEntityOnRemoval(pragma::ecs::BaseEntity &ent, pragma::ecs::BaseEntity &entOther, Bool remove) { ent.RemoveEntityOnRemoval(&entOther, remove); }
void Lua::Entity::RemoveEntityOnRemoval(pragma::ecs::BaseEntity &ent, pragma::ecs::BaseEntity &entOther) { RemoveEntityOnRemoval(ent, entOther, true); }

float Lua::Entity::GetAirDensity(pragma::ecs::BaseEntity &ent)
{
	return 1.225f; // Placeholder
}
