// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"
#include "util_enum_flags.hpp"

module pragma.shared;

import :entities.base_entity;
import :game.game;

namespace Lua {
	bool get_bullet_master(pragma::ecs::BaseEntity &ent);
	pragma::AnimationEvent get_animation_event(lua::State *l, int32_t tArgs, uint32_t eventId);
};
bool Lua::get_bullet_master(pragma::ecs::BaseEntity &ent)
{
	auto bMaster = true;
	if(ent.IsWeapon()) {
		auto &wepComponent = *ent.GetWeaponComponent();
		auto *ownerComponent = wepComponent.GetOwnerComponent();
		auto *owner = (ownerComponent != nullptr) ? ownerComponent->GetOwner() : nullptr;
		if(owner != nullptr && owner->IsPlayer()) {
			auto plComponent = owner->GetPlayerComponent();
			if(ent.GetNetworkState()->IsServer() || plComponent.expired() || !plComponent->IsLocalPlayer())
				bMaster = false; // Assume that shot originated from other client
		}
	}
	return bMaster;
}
pragma::AnimationEvent Lua::get_animation_event(lua::State *l, int32_t tArgs, uint32_t eventId)
{
	CheckTable(l, tArgs);
	pragma::AnimationEvent ev {};
	ev.eventID = static_cast<pragma::AnimationEvent::Type>(eventId);
	auto numArgs = GetObjectLength(l, tArgs);
	for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
		PushInt(l, i + 1); /* 1 */
		GetTableValue(l, tArgs);

		auto *arg = CheckString(l, -1);
		ev.arguments.push_back(arg);

		Pop(l, 1);
	}
	return ev;
}

static int lua_match_component_member_reference(lua::State *l, int index) { return Lua::IsString(l, index) ? 0 : luabind::no_match; }
static pragma::ComponentMemberReference lua_to_component_member_reference(lua::State *l, int index) { return pragma::ComponentMemberReference {Lua::CheckString(l, index)}; }

template<uint32_t N>
using ComponentMemberReferencePolicy = luabind::generic_policy<N, pragma::ComponentMemberReference, &lua_match_component_member_reference, &lua_to_component_member_reference>;

/*template<uint32_t N>
	using UniversalReferencePolicy = luabind::generic_policy<N,util::Uuid,[](lua::State *l,int index) -> int {
		return Lua::IsString(l,index) ? 0 : luabind::no_match;
	},[](lua::State *l,int index) -> pragma::util::Uuid {
		return pragma::util::uuid_string_to_bytes(Lua::CheckString(l,index));
	}>;
*/

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, EntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, EntityUComponentRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, EntityUComponentMemberRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, MultiEntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, MultiEntityUComponentRef);

// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
// DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::game, ValueDriver);
#endif

namespace pragma {
	enum class BvhIntersectionFlags : uint32_t {
		None = 0u,
		ReturnPrimitives = 1u,
		ReturnMeshes = ReturnPrimitives << 1u,
	};
	using namespace pragma::math::scoped_enum::bitwise;
}
REGISTER_ENUM_FLAGS(pragma::BvhIntersectionFlags)
using IntersectionTestResult = Lua::type<std::pair<bool, Lua::var<std::optional<std::vector<uint64_t>>, std::optional<std::vector<pragma::MeshIntersectionInfo::MeshInfo>>>>>;
static IntersectionTestResult bvh_intersection_test(lua::State *l, const std::function<bool(pragma::IntersectionInfo *)> &fTest, pragma::BvhIntersectionFlags flags)
{
	if(!pragma::math::is_flag_set(flags, pragma::BvhIntersectionFlags::ReturnPrimitives | pragma::BvhIntersectionFlags::ReturnMeshes)) {
		auto res = fTest(nullptr);
		return luabind::object {l, std::pair<bool, std::optional<std::vector<uint64_t>>> {res, {}}};
	}
	if(pragma::math::is_flag_set(flags, pragma::BvhIntersectionFlags::ReturnPrimitives)) {
		pragma::PrimitiveIntersectionInfo info {};
		auto res = fTest(&info);
		if(!res)
			return luabind::object {l, std::pair<bool, std::optional<std::vector<uint64_t>>> {res, {}}};
		return luabind::object {l, std::pair<bool, std::optional<std::vector<uint64_t>>> {res, std::move(info.primitives)}};
	}
	pragma::MeshIntersectionInfo info {};
	auto res = fTest(&info);
	if(!res)
		return luabind::object {l, std::pair<bool, std::optional<std::vector<uint64_t>>> {res, {}}};
	return luabind::object {l, std::pair<bool, std::optional<std::vector<pragma::MeshIntersectionInfo::MeshInfo>>> {res, std::move(info.meshInfos)}};
}

template<typename TResult, bool (pragma::MetaRigComponent::*GetValue)(pragma::animation::MetaRigBoneType, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_meta_bone_value(const pragma::MetaRigComponent &metaC, pragma::animation::MetaRigBoneType bone, pragma::math::CoordinateSpace space)
{
	TResult result;
	if(!(metaC.*GetValue)(bone, result, space))
		return {};
	return result;
}
template<typename TResult, bool (pragma::MetaRigComponent::*GetValue)(pragma::animation::MetaRigBoneType, TResult &, pragma::math::CoordinateSpace) const>
std::optional<TResult> get_meta_bone_value_ls(const pragma::MetaRigComponent &metaC, pragma::animation::MetaRigBoneType bone)
{
	return get_meta_bone_value<TResult, GetValue>(metaC, bone, pragma::math::CoordinateSpace::Local);
}

void pragma::Game::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	LuaCore::register_entity_component_classes(GetLuaState(), entsMod);

	auto classDefEntRef = luabind::class_<EntityURef>("UniversalEntityReference");
	classDefEntRef.def(luabind::constructor<const ecs::BaseEntity &>());
	classDefEntRef.def(luabind::constructor<const std::string &>());
	classDefEntRef.def(luabind::tostring(luabind::self));
	classDefEntRef.def("GetEntity", static_cast<ecs::BaseEntity *(EntityURef::*)(Game &)>(&EntityURef::GetEntity));
	classDefEntRef.def(
	  "GetUuid", +[](EntityURef &uref) -> std::optional<Lua::util::Uuid> {
		  auto uuid = uref.GetUuid();
		  return uuid.has_value() ? Lua::util::Uuid {*uuid} : std::optional<Lua::util::Uuid> {};
	  });
	classDefEntRef.def("GetClassOrName", static_cast<std::optional<std::string> (EntityURef::*)() const>(&EntityURef::GetClassOrName));
	entsMod[classDefEntRef];
	pragma::LuaCore::define_custom_constructor<EntityURef, +[](const Lua::util::Uuid &uuid) -> EntityURef { return EntityURef {uuid.value}; }, const Lua::util::Uuid &>(GetLuaState());

	auto classDefCompRef = luabind::class_<EntityUComponentRef, EntityURef>("UniversalComponentReference");
	classDefCompRef.def(luabind::constructor<const std::string &, ComponentId>());
	classDefCompRef.def(luabind::constructor<const std::string &, const std::string &>());
	classDefCompRef.def(luabind::constructor<const ecs::BaseEntity &, ComponentId>());
	classDefCompRef.def(luabind::tostring(luabind::self));
	classDefCompRef.def("GetComponent", static_cast<BaseEntityComponent *(EntityUComponentRef::*)(Game &)>(&EntityUComponentRef::GetComponent));
	classDefCompRef.def("GetComponentId", &EntityUComponentMemberRef::GetComponentId);
	classDefCompRef.def(
	  "GetComponentName", +[](const EntityUComponentRef &ref) -> std::optional<std::string> {
		  auto *name = ref.GetComponentName();
		  if(!name)
			  return {};
		  return *name;
	  });
	entsMod[classDefCompRef];
	pragma::LuaCore::define_custom_constructor<EntityUComponentRef, +[](const Lua::util::Uuid &uuid, ComponentId componentId) -> EntityUComponentRef { return EntityUComponentRef {uuid.value, componentId}; }, const Lua::util::Uuid &, ComponentId>(
	  GetLuaState());
	pragma::LuaCore::define_custom_constructor<EntityUComponentRef, +[](const Lua::util::Uuid &uuid, const std::string &componentType) -> EntityUComponentRef { return EntityUComponentRef {uuid.value, componentType}; }, const Lua::util::Uuid &, const std::string &>(
	  GetLuaState());

	auto classDefMemRef = luabind::class_<EntityUComponentMemberRef, luabind::bases<EntityUComponentRef, EntityURef>>("UniversalMemberReference");
	classDefMemRef.def(luabind::constructor<const std::string &, ComponentId, const std::string &>());
	classDefMemRef.def(luabind::constructor<const std::string &, const std::string &, const std::string &>());
	classDefMemRef.def(luabind::constructor<const ecs::BaseEntity &, ComponentId, const std::string &>());
	classDefMemRef.def(luabind::constructor<const ecs::BaseEntity &, const std::string &, const std::string &>());
	classDefMemRef.def(luabind::constructor<const std::string &>());
	classDefMemRef.def(luabind::constructor<>());
	classDefMemRef.def(luabind::tostring(luabind::self));
	classDefMemRef.def("GetMemberInfo", &EntityUComponentMemberRef::GetMemberInfo);
	classDefMemRef.def(
	  "GetMemberIndex", +[](Game &game, const EntityUComponentMemberRef &ref) -> std::optional<ComponentMemberIndex> {
		  if(ref.GetMemberIndex() == INVALID_COMPONENT_ID)
			  ref.GetMemberInfo(game);
		  auto idx = ref.GetMemberIndex();
		  return (idx != INVALID_COMPONENT_ID) ? idx : std::optional<ComponentMemberIndex> {};
	  });
	classDefMemRef.def(
	  "GetMemberName", +[](Game &game, const EntityUComponentMemberRef &ref) -> std::optional<std::string> {
		  if(ref.GetMemberIndex() == INVALID_COMPONENT_ID)
			  ref.GetMemberInfo(game);
		  auto name = ref.GetMemberName();
		  return !name.empty() ? name : std::optional<std::string> {};
	  });
	classDefMemRef.def(
	  "GetPath", +[](Game &game, const EntityUComponentMemberRef &ref) -> std::optional<std::string> {
		  auto *c = ref.GetComponent(game);
		  auto *cInfo = c ? c->GetComponentInfo() : nullptr;
		  auto &memberName = ref.GetMemberName();
		  if(!cInfo || memberName.empty())
			  return {};
		  std::string name = "pragma:game/entity/ec/" + std::string {*cInfo->name} + "/" + memberName;
		  auto uuid = ref.GetUuid();
		  if(uuid.has_value())
			  name += "?entity_uuid=" + util::uuid_to_string(*uuid);
		  return name;
	  });
	classDefMemRef.def(
	  "GetValue", +[](lua::State *l, Game &game, const EntityUComponentMemberRef &ref) {
		  auto *c = ref.GetComponent(game);
		  auto *memberInfo = ref.GetMemberInfo(game);
		  if(!c || !memberInfo)
			  return std::optional<Lua::udm_type> {};
		  return LuaCore::get_member_value(l, const_cast<BaseEntityComponent &>(*c), *memberInfo);
	  });
	entsMod[classDefMemRef];
	pragma::LuaCore::define_custom_constructor<EntityUComponentMemberRef,
	  +[](const Lua::util::Uuid &uuid, ComponentId componentId, const std::string &memberName) -> EntityUComponentMemberRef { return EntityUComponentMemberRef {uuid.value, componentId, memberName}; }, const Lua::util::Uuid &, ComponentId,
	  const std::string &>(GetLuaState());
	pragma::LuaCore::define_custom_constructor<EntityUComponentMemberRef,
	  +[](const Lua::util::Uuid &uuid, const std::string &componentType, const std::string &memberName) -> EntityUComponentMemberRef { return EntityUComponentMemberRef {uuid.value, componentType, memberName}; }, const Lua::util::Uuid &, const std::string &,
	  const std::string &>(GetLuaState());

	auto classDefMultiEntRef = luabind::class_<MultiEntityURef>("MultiUniversalEntityReference");
	classDefMultiEntRef.def(luabind::constructor<const ecs::BaseEntity &>());
	classDefMultiEntRef.def(luabind::constructor<const std::string &>());
	classDefMultiEntRef.def("FindEntities", &MultiEntityURef::FindEntities);
	entsMod[classDefMultiEntRef];
	pragma::LuaCore::define_custom_constructor<MultiEntityURef, +[](const Lua::util::Uuid &uuid) -> MultiEntityURef { return MultiEntityURef {uuid.value}; }, const Lua::util::Uuid &>(GetLuaState());

	auto defVelocity = pragma::LuaCore::create_entity_component_class<VelocityComponent, BaseEntityComponent>("VelocityComponent");
	defVelocity.def("GetVelocity", &VelocityComponent::GetVelocity, luabind::copy_policy<0> {});
	defVelocity.def("SetVelocity", &VelocityComponent::SetVelocity);
	defVelocity.def("AddVelocity", &VelocityComponent::AddVelocity);
	defVelocity.def("GetAngularVelocity", &VelocityComponent::GetAngularVelocity, luabind::copy_policy<0> {});
	defVelocity.def("SetAngularVelocity", &VelocityComponent::SetAngularVelocity);
	defVelocity.def("AddAngularVelocity", &VelocityComponent::AddAngularVelocity);
	defVelocity.def("GetLocalAngularVelocity", &VelocityComponent::GetLocalAngularVelocity);
	defVelocity.def("SetLocalAngularVelocity", &VelocityComponent::SetLocalAngularVelocity);
	defVelocity.def("AddLocalAngularVelocity", &VelocityComponent::AddLocalAngularVelocity);
	defVelocity.def("GetLocalVelocity", &VelocityComponent::GetLocalVelocity);
	defVelocity.def("SetLocalVelocity", &VelocityComponent::SetLocalVelocity);
	defVelocity.def("AddLocalVelocity", &VelocityComponent::AddLocalVelocity);
	defVelocity.def("GetVelocityProperty", &VelocityComponent::GetVelocityProperty);
	defVelocity.def("GetAngularVelocityProperty", &VelocityComponent::GetAngularVelocityProperty);
	entsMod[defVelocity];

	auto defActionC = pragma::LuaCore::create_entity_component_class<ActionInputControllerComponent, BaseEntityComponent>("ActionInputControllerComponent");
	defActionC.def("GetActionInput", &ActionInputControllerComponent::GetActionInput);
	defActionC.def("GetActionInputs", &ActionInputControllerComponent::GetActionInputs);
	defActionC.def("GetActionInputAxisMagnitude", &ActionInputControllerComponent::GetActionInputAxisMagnitude);
	defActionC.def("SetActionInputAxisMagnitude", &ActionInputControllerComponent::SetActionInputAxisMagnitude);
	defActionC.def("SetActionInput", static_cast<void (ActionInputControllerComponent ::*)(Action, bool, bool)>(&ActionInputControllerComponent::SetActionInput));
	defActionC.def("SetActionInput", static_cast<void (ActionInputControllerComponent ::*)(Action, bool, float)>(&ActionInputControllerComponent::SetActionInput));
	defActionC.def("SetActionInput", static_cast<void (ActionInputControllerComponent ::*)(Action, bool, float)>(&ActionInputControllerComponent::SetActionInput), luabind::default_parameter_policy<4, 1.f> {});
	defActionC.add_static_constant("EVENT_HANDLE_ACTION_INPUT", actionInputControllerComponent::EVENT_HANDLE_ACTION_INPUT);
	entsMod[defActionC];

	auto defInputMovementC = pragma::LuaCore::create_entity_component_class<InputMovementControllerComponent, BaseEntityComponent>("InputMovementControllerComponent");
	defInputMovementC.def("GetActionInputController", static_cast<ActionInputControllerComponent *(InputMovementControllerComponent ::*)()>(&InputMovementControllerComponent::GetActionInputController));
	defInputMovementC.def("SetActionInputController", &InputMovementControllerComponent::SetActionInputController);
	entsMod[defInputMovementC];

	auto defMetaRig = pragma::LuaCore::create_entity_component_class<MetaRigComponent, BaseEntityComponent>("MetaRigComponent");
	defMetaRig.def("GetBonePose", &get_meta_bone_value<math::ScaledTransform, &MetaRigComponent::GetBonePose>);
	defMetaRig.def("GetBonePos", &get_meta_bone_value<Vector3, &MetaRigComponent::GetBonePos>);
	defMetaRig.def("GetBoneRot", &get_meta_bone_value<Quat, &MetaRigComponent::GetBoneRot>);
	defMetaRig.def("GetBoneScale", &get_meta_bone_value<Vector3, &MetaRigComponent::GetBoneScale>);

	defMetaRig.def("GetBonePose", &get_meta_bone_value_ls<math::ScaledTransform, &MetaRigComponent::GetBonePose>);
	defMetaRig.def("GetBonePos", &get_meta_bone_value_ls<Vector3, &MetaRigComponent::GetBonePos>);
	defMetaRig.def("GetBoneRot", &get_meta_bone_value_ls<Quat, &MetaRigComponent::GetBoneRot>);
	defMetaRig.def("GetBoneScale", &get_meta_bone_value_ls<Vector3, &MetaRigComponent::GetBoneScale>);

	defMetaRig.def("SetBonePose", &MetaRigComponent::SetBonePose, luabind::default_parameter_policy<4, math::CoordinateSpace::Local> {});
	defMetaRig.def("SetBonePos", &MetaRigComponent::SetBonePos, luabind::default_parameter_policy<4, math::CoordinateSpace::Local> {});
	defMetaRig.def("SetBoneRot", &MetaRigComponent::SetBoneRot, luabind::default_parameter_policy<4, math::CoordinateSpace::Local> {});
	defMetaRig.def("SetBoneScale", &MetaRigComponent::SetBoneScale, luabind::default_parameter_policy<4, math::CoordinateSpace::Local> {});
	entsMod[defMetaRig];

	auto defBoneMerge = pragma::LuaCore::create_entity_component_class<BoneMergeComponent, BaseEntityComponent>("BoneMergeComponent");
	defBoneMerge.scope[luabind::def("can_merge", &BoneMergeComponent::can_merge)];
	defBoneMerge.scope[luabind::def("can_merge", &BoneMergeComponent::can_merge, luabind::default_parameter_policy<3, false> {})];
	defBoneMerge.add_static_constant("EVENT_ON_TARGET_CHANGED", boneMergeComponent::EVENT_ON_TARGET_CHANGED);
	defBoneMerge.def("SetTarget", &BoneMergeComponent::SetTarget);
	defBoneMerge.def("GetTarget", &BoneMergeComponent::GetTarget);
	entsMod[defBoneMerge];

	auto defFlexMerge = pragma::LuaCore::create_entity_component_class<FlexMergeComponent, BaseEntityComponent>("FlexMergeComponent");
	defFlexMerge.scope[luabind::def("can_merge", &FlexMergeComponent::can_merge)];
	defFlexMerge.add_static_constant("EVENT_ON_TARGET_CHANGED", flexMergeComponent::EVENT_ON_TARGET_CHANGED);
	defFlexMerge.def("SetTarget", &FlexMergeComponent::SetTarget);
	defFlexMerge.def("GetTarget", &FlexMergeComponent::GetTarget);
	entsMod[defFlexMerge];

	auto defIntersectionHandler = pragma::LuaCore::create_entity_component_class<IntersectionHandlerComponent, BaseEntityComponent>("IntersectionHandlerComponent");
	defIntersectionHandler.def(
	  "IntersectionTest2", +[](IntersectionHandlerComponent &c, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) {
		  auto t = std::chrono::steady_clock::now();
		  c.IntersectionTest(origin, dir, math::CoordinateSpace::Object, minDist, maxDist);
		  auto dt = std::chrono::steady_clock::now() - t;
		  std::cout << "Internal2: " << (dt.count() / 1'000'000.0) << "ms" << std::endl;
	  });
	defIntersectionHandler.def(
	  "IntersectionTest3", +[](lua::State *l, size_t tStart) {
		  auto t = std::chrono::steady_clock::now();
		  auto dt = std::chrono::steady_clock::now().time_since_epoch().count() - tStart;
		  std::cout << "Lua Overhead: " << (dt / 1'000'000.0) << "ms" << std::endl;
	  });
	defIntersectionHandler.def(
	  "IntersectionTest", +[](IntersectionHandlerComponent &c, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) -> std::optional<HitInfo> { return c.IntersectionTest(origin, dir, math::CoordinateSpace::Object, minDist, maxDist); });
	// defBvh.def("IntersectionTest", static_cast<std::optional<pragma::bvh::HitInfo> (pragma::IntersectionHandlerComponent::*)(const Vector3 &, const Vector3 &, float, float) const>(&pragma::IntersectionHandlerComponent::IntersectionTest));
	defIntersectionHandler.def("IntersectionTestAabb", static_cast<bool (IntersectionHandlerComponent::*)(const Vector3 &, const Vector3 &) const>(&IntersectionHandlerComponent::IntersectionTestAabb));
	defIntersectionHandler.def(
	  "IntersectionTestAabb", +[](lua::State *l, const IntersectionHandlerComponent &bvhC, const Vector3 &min, const Vector3 &max, BvhIntersectionFlags flags) -> IntersectionTestResult {
		  return bvh_intersection_test(l, [&bvhC, &min, &max](IntersectionInfo *info) { return info ? bvhC.IntersectionTestAabb(min, max, *info) : bvhC.IntersectionTestAabb(min, max); }, flags);
	  });
	defIntersectionHandler.def("IntersectionTestKDop", static_cast<bool (IntersectionHandlerComponent::*)(const std::vector<math::Plane> &) const>(&IntersectionHandlerComponent::IntersectionTestKDop));
	defIntersectionHandler.def(
	  "IntersectionTestKDop", +[](lua::State *l, const IntersectionHandlerComponent &bvhC, const std::vector<math::Plane> &planes, BvhIntersectionFlags flags) -> IntersectionTestResult {
		  return bvh_intersection_test(l, [&bvhC, &planes](IntersectionInfo *info) { return info ? bvhC.IntersectionTestKDop(planes, *info) : bvhC.IntersectionTestKDop(planes); }, flags);
	  });
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_NONE", math::to_integral(BvhIntersectionFlags::None));
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_BIT_RETURN_PRIMITIVES", math::to_integral(BvhIntersectionFlags::ReturnPrimitives));
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_BIT_RETURN_MESHES", math::to_integral(BvhIntersectionFlags::ReturnMeshes));

	auto defBvhHitInfo = luabind::class_<HitInfo>("HitInfo");
	defBvhHitInfo.def_readonly("mesh", &HitInfo::mesh);
	defBvhHitInfo.def_readonly("entity", &HitInfo::entity);
	defBvhHitInfo.property(
	  "entity",
	  +[](lua::State *l, HitInfo &info) {
		  if(info.entity.expired())
			  Lua::PushNil(l);
		  else
			  info.entity->PushLuaObject(l);
	  },
	  +[](HitInfo &info, ecs::BaseEntity *ent) { info.entity = ent ? ent->GetHandle() : EntityHandle {}; });
	defBvhHitInfo.def_readonly("primitiveIndex", &HitInfo::primitiveIndex);
	defBvhHitInfo.def_readonly("distance", &HitInfo::distance);
	defBvhHitInfo.def_readonly("t", &HitInfo::t);
	defBvhHitInfo.def_readonly("u", &HitInfo::u);
	defBvhHitInfo.def_readonly("v", &HitInfo::v);
	defBvhHitInfo.def(
	  "CalcHitNormal", +[](const HitInfo &hitInfo) -> std::optional<Vector3> {
		  if(!hitInfo.mesh)
			  return {};
		  auto idx = hitInfo.primitiveIndex * 3;
		  auto vIdx0 = hitInfo.mesh->GetIndex(idx);
		  auto vIdx1 = hitInfo.mesh->GetIndex(idx + 1);
		  auto vIdx2 = hitInfo.mesh->GetIndex(idx + 2);
		  if(!vIdx0.has_value() || !vIdx1.has_value() || !vIdx2.has_value())
			  return {};
		  auto n0 = hitInfo.mesh->GetVertexNormal(*vIdx0);
		  auto n1 = hitInfo.mesh->GetVertexNormal(*vIdx1);
		  auto n2 = hitInfo.mesh->GetVertexNormal(*vIdx2);
		  auto n = hitInfo.t * n0 + hitInfo.u * n1 + hitInfo.v * n2;
		  uvec::normalize(&n);
		  return n;
	  });
	defBvhHitInfo.def(
	  "CalcHitUv", +[](const HitInfo &hitInfo) -> std::optional<Vector2> {
		  if(!hitInfo.mesh)
			  return {};
		  auto idx = hitInfo.primitiveIndex * 3;
		  auto vIdx0 = hitInfo.mesh->GetIndex(idx);
		  auto vIdx1 = hitInfo.mesh->GetIndex(idx + 1);
		  auto vIdx2 = hitInfo.mesh->GetIndex(idx + 2);
		  if(!vIdx0.has_value() || !vIdx1.has_value() || !vIdx2.has_value())
			  return {};
		  auto uv0 = hitInfo.mesh->GetVertexUV(*vIdx0);
		  auto uv1 = hitInfo.mesh->GetVertexUV(*vIdx1);
		  auto uv2 = hitInfo.mesh->GetVertexUV(*vIdx2);
		  auto u = hitInfo.u;
		  auto v = hitInfo.v;
		  return (1.f - (hitInfo.u + hitInfo.v)) * uv0 + hitInfo.u * uv1 + hitInfo.v * uv2;
	  });
	defIntersectionHandler.scope[defBvhHitInfo];
	entsMod[defIntersectionHandler];

	auto defGlobal = pragma::LuaCore::create_entity_component_class<GlobalNameComponent, BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName", &GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName", &GlobalNameComponent::SetGlobalName);
	entsMod[defGlobal];

	auto defMovement = pragma::LuaCore::create_entity_component_class<MovementComponent, BaseEntityComponent>("MovementComponent");
	defMovement.def("GetMoveVelocity", &MovementComponent::GetMoveVelocity, luabind::copy_policy<0> {});
	defMovement.def("GetRelativeVelocity", &MovementComponent::GetLocalVelocity);
	defMovement.def("SetSpeed", &MovementComponent::SetSpeed);
	defMovement.def("GetSpeed", &MovementComponent::GetSpeed);
	defMovement.def("SetAirModifier", &MovementComponent::SetAirModifier);
	defMovement.def("GetAirModifier", &MovementComponent::GetAirModifier);
	defMovement.def("SetAcceleration", &MovementComponent::SetAcceleration);
	defMovement.def("GetAcceleration", &MovementComponent::GetAcceleration);
	defMovement.def("SetAccelerationRampUpTime", &MovementComponent::SetAccelerationRampUpTime);
	defMovement.def("GetAccelerationRampUpTime", &MovementComponent::GetAccelerationRampUpTime);
	defMovement.def("SetDirection", &MovementComponent::SetDirection);
	defMovement.def("GetDirection", &MovementComponent::GetDirection);
	defMovement.def("SetDirectionMagnitude", &MovementComponent::SetDirectionMagnitude);
	defMovement.def("GetDirectionMagnitude", &MovementComponent::GetDirectionMagnitude);
	defMovement.add_static_constant("EVENT_ON_UPDATE_MOVEMENT", movementComponent::EVENT_ON_UPDATE_MOVEMENT);
	defMovement.add_static_constant("MOVE_DIRECTION_FORWARD", math::to_integral(MovementComponent::MoveDirection::Forward));
	defMovement.add_static_constant("MOVE_DIRECTION_RIGHT", math::to_integral(MovementComponent::MoveDirection::Right));
	defMovement.add_static_constant("MOVE_DIRECTION_BACKWARD", math::to_integral(MovementComponent::MoveDirection::Backward));
	defMovement.add_static_constant("MOVE_DIRECTION_LEFT", math::to_integral(MovementComponent::MoveDirection::Left));
	entsMod[defMovement];

	auto defOrientation = pragma::LuaCore::create_entity_component_class<OrientationComponent, BaseEntityComponent>("OrientationComponent");
	defOrientation.def("GetUpDirection", &OrientationComponent::GetUpDirection, luabind::copy_policy<0> {});
	defOrientation.def("SetUpDirection", &OrientationComponent::SetUpDirection);
	defOrientation.def("GetUpDirectionProperty", &OrientationComponent::GetUpDirectionProperty);
	defOrientation.def(
	  "GetOrientationAxes", +[](lua::State *l, OrientationComponent &hEntity) {
		  Vector3 forward, right, up;
		  hEntity.GetOrientationAxes(&forward, &right, &up);
		  Lua::Push<Vector3>(l, forward);
		  Lua::Push<Vector3>(l, right);
		  Lua::Push<Vector3>(l, up);
	  });
	entsMod[defOrientation];

	auto defComposite = pragma::LuaCore::create_entity_component_class<ecs::CompositeComponent, BaseEntityComponent>("CompositeComponent");
	defComposite.def("ClearEntities", &ecs::CompositeComponent::ClearEntities);
	defComposite.def("ClearEntities", &ecs::CompositeComponent::ClearEntities, luabind::default_parameter_policy<2, true> {});
	defComposite.def(
	  "ClearEntities", +[](lua::State *l, ecs::CompositeComponent &hComponent, const std::string &groupName) {
		  auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities();
	  });
	defComposite.def(
	  "ClearEntities", +[](lua::State *l, ecs::CompositeComponent &hComponent, const std::string &groupName, bool safely) {
		  auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities(safely);
	  });
	defComposite.def(
	  "GetEntities", +[](lua::State *l, ecs::CompositeComponent &hComponent) -> luabind::tableT<ecs::BaseEntity> {
		  auto &ents = hComponent.GetRootCompositeGroup().GetEntities();
		  auto tEnts = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &pair : ents) {
			  if(!pair.second.valid())
				  continue;
			  tEnts[idx++] = pair.second.get()->GetLuaObject();
		  }
		  return tEnts;
	  });
	defComposite.def(
	  "GetEntities", +[](lua::State *l, ecs::CompositeComponent &hComponent, const std::string &groupName) -> luabind::tableT<ecs::BaseEntity> {
		  auto tEnts = luabind::newtable(l);
		  auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		  if(!group)
			  return tEnts;
		  auto &ents = group->GetEntities();
		  int32_t idx = 1;
		  for(auto &pair : ents) {
			  if(!pair.second.valid())
				  continue;
			  tEnts[idx++] = pair.second.get()->GetLuaObject();
		  }
		  return tEnts;
	  });
	defComposite.def("GetRootGroup", static_cast<ecs::CompositeGroup &(ecs::CompositeComponent::*)()>(&ecs::CompositeComponent::GetRootCompositeGroup));
	defComposite.def("AddEntity", +[](lua::State *l, ecs::CompositeComponent &hComponent, ecs::BaseEntity &ent) { hComponent.GetRootCompositeGroup().AddEntity(ent); });
	defComposite.def("AddEntity", +[](lua::State *l, ecs::CompositeComponent &hComponent, ecs::BaseEntity &ent, const std::string &groupName) { hComponent.GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(ent); });
	auto defCompositeGroup = luabind::class_<ecs::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity", &ecs::CompositeGroup::AddEntity);
	defCompositeGroup.def("AddEntity", +[](lua::State *l, ecs::CompositeGroup &hComponent, ecs::BaseEntity &ent, const std::string &groupName) { hComponent.AddChildGroup(groupName).AddEntity(ent); });
	defCompositeGroup.def("RemoveEntity", &ecs::CompositeGroup::RemoveEntity);
	defCompositeGroup.def("ClearEntities", &ecs::CompositeGroup::ClearEntities);
	defCompositeGroup.def("ClearEntities", &ecs::CompositeGroup::ClearEntities, luabind::default_parameter_policy<2, true> {});
	defCompositeGroup.def(
	  "ClearEntities", +[](lua::State *l, ecs::CompositeGroup &hComponent, const std::string &groupName) {
		  auto *group = hComponent.FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities();
	  });
	defCompositeGroup.def(
	  "ClearEntities", +[](lua::State *l, ecs::CompositeGroup &hComponent, const std::string &groupName, bool safely) {
		  auto *group = hComponent.FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities(safely);
	  });
	defCompositeGroup.def(
	  "GetEntities", +[](lua::State *l, ecs::CompositeGroup &hComponent) -> luabind::tableT<ecs::BaseEntity> {
		  auto &ents = hComponent.GetEntities();
		  auto tEnts = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &pair : ents) {
			  if(!pair.second.valid())
				  continue;
			  tEnts[idx++] = pair.second.get()->GetLuaObject();
		  }
		  return tEnts;
	  });
	defCompositeGroup.def("AddChildGroup", &ecs::CompositeGroup::AddChildGroup);
	defCompositeGroup.def(
	  "GetChildGroups", +[](lua::State *l, ecs::CompositeGroup &hComponent, const std::string &name) -> luabind::tableT<ecs::CompositeGroup> {
		  auto t = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &childGroup : hComponent.GetChildGroups())
			  t[idx++] = childGroup.get();
		  return t;
	  });
	defComposite.scope[defCompositeGroup];
	entsMod[defComposite];

	auto defAnimated2 = pragma::LuaCore::create_entity_component_class<PanimaComponent, BaseEntityComponent>("PanimaComponent");
	defAnimated2.scope[luabind::def("parse_component_channel_path", &PanimaComponent::ParseComponentChannelPath)];
	defAnimated2.def("ReloadAnimation", static_cast<void (PanimaComponent::*)()>(&PanimaComponent::ReloadAnimation));
	defAnimated2.def("SetPlaybackRate", &PanimaComponent::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate", &PanimaComponent::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty", &PanimaComponent::GetPlaybackRateProperty);
	defAnimated2.def("GetCurrentTime", &PanimaComponent::GetCurrentTime);
	defAnimated2.def("SetCurrentTime", &PanimaComponent::SetCurrentTime);
	defAnimated2.def("GetCurrentTimeFraction", &PanimaComponent::GetCurrentTimeFraction);
	defAnimated2.def("SetCurrentTimeFraction", &PanimaComponent::SetCurrentTimeFraction);
	defAnimated2.def("SetPropertyEnabled", &PanimaComponent::SetPropertyEnabled);
	defAnimated2.def("IsPropertyEnabled", &PanimaComponent::IsPropertyEnabled);
	defAnimated2.def("IsPropertyAnimated", &PanimaComponent::IsPropertyAnimated);
	defAnimated2.def("UpdateAnimationChannelSubmitters", &PanimaComponent::UpdateAnimationChannelSubmitters);
	defAnimated2.def("ClearAnimationManagers", &PanimaComponent::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager", &PanimaComponent::AddAnimationManager);
	defAnimated2.def("AddAnimationManager", &PanimaComponent::AddAnimationManager, luabind::default_parameter_policy<3, int32_t {0}> {});
	defAnimated2.def("RemoveAnimationManager", static_cast<void (PanimaComponent::*)(const panima::AnimationManager &)>(&PanimaComponent::RemoveAnimationManager));
	defAnimated2.def("RemoveAnimationManager", static_cast<void (PanimaComponent::*)(const std::string_view &)>(&PanimaComponent::RemoveAnimationManager));
	defAnimated2.def(
	  "GetAnimationManagers", +[](lua::State *l, PanimaComponent &hComponent) -> luabind::tableT<panima::AnimationManager> {
		  auto t = luabind::newtable(l);
		  auto &animManagers = hComponent.GetAnimationManagers();
		  for(auto &amData : animManagers)
			  t[amData->name] = amData->animationManager;
		  return t;
	  });
	defAnimated2.def("GetAnimationManager", +[](lua::State *l, PanimaComponent &hComponent, const std::string &name) { return hComponent.GetAnimationManager(name); });
	defAnimated2.def("PlayAnimation", &PanimaComponent::PlayAnimation);
	defAnimated2.def("ReloadAnimation", static_cast<void (PanimaComponent::*)(panima::AnimationManager &)>(&PanimaComponent::ReloadAnimation));
	defAnimated2.def("AdvanceAnimations", &PanimaComponent::AdvanceAnimations);
	defAnimated2.def("DebugPrint", static_cast<void (PanimaComponent::*)()>(&PanimaComponent::DebugPrint));
	defAnimated2.def(
	  "GetRawAnimatedPropertyValue", +[](lua::State *l, PanimaComponent &c, panima::AnimationManager &manager, const std::string &propName, udm::Type type) -> Lua::opt<Lua::udm_ng> {
		  luabind::object r = Lua::nil;
		  udm::visit_ng(type, [l, &c, &manager, &propName, type, &r](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::is_animatable_type_v<T>) {
				  T value;
				  if(c.GetRawAnimatedPropertyValue(manager, propName, type, &value))
					  r = luabind::object {l, value};
			  }
		  });
		  return r;
	  });
	defAnimated2.def(
	  "GetRawPropertyValue", +[](lua::State *l, PanimaComponent &c, panima::AnimationManager &manager, const std::string &propName, udm::Type type) -> Lua::opt<Lua::udm_ng> {
		  luabind::object r = Lua::nil;
		  udm::visit_ng(type, [l, &c, &manager, &propName, type, &r](auto tag) {
			  using T = typename decltype(tag)::type;
			  if constexpr(pragma::is_animatable_type_v<T>) {
				  T value;
				  if(c.GetRawPropertyValue(manager, propName, type, &value))
					  r = luabind::object {l, value};
			  }
		  });
		  return r;
	  });
	defAnimated2.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", panimaComponent::EVENT_HANDLE_ANIMATION_EVENT);
	defAnimated2.add_static_constant("EVENT_ON_PLAY_ANIMATION", panimaComponent::EVENT_ON_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", panimaComponent::EVENT_ON_ANIMATION_COMPLETE);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_START", panimaComponent::EVENT_ON_ANIMATION_START);
	defAnimated2.add_static_constant("EVENT_MAINTAIN_ANIMATIONS", panimaComponent::EVENT_MAINTAIN_ANIMATIONS);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED", panimaComponent::EVENT_ON_ANIMATIONS_UPDATED);
	defAnimated2.add_static_constant("EVENT_PLAY_ANIMATION", panimaComponent::EVENT_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_TRANSLATE_ANIMATION", panimaComponent::EVENT_TRANSLATE_ANIMATION);
	defAnimated2.add_static_constant("EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER", panimaComponent::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER);
	entsMod[defAnimated2];

	auto defDriverC = pragma::LuaCore::create_entity_component_class<AnimationDriverComponent, BaseEntityComponent>("AnimationDriverComponent");
	defDriverC.def("SetExpression", &AnimationDriverComponent::SetExpression);
	defDriverC.def("GetExpression", &AnimationDriverComponent::GetExpression);
	defDriverC.def("AddReference", &AnimationDriverComponent::AddReference);
	defDriverC.def("GetConstants", &AnimationDriverComponent::GetConstants);
	defDriverC.def("GetReferences", &AnimationDriverComponent::GetReferences);
	defDriverC.def("SetDrivenObject", &AnimationDriverComponent::SetDrivenObject);
	defDriverC.def("GetDrivenObject", &AnimationDriverComponent::GetDrivenObject);
	defDriverC.def("AddConstant", static_cast<void (AnimationDriverComponent::*)(const std::string &, const udm::PProperty &)>(&AnimationDriverComponent::AddConstant));

	auto defDriver = luabind::class_<game::ValueDriver>("Driver");
	defDriver.def(luabind::tostring(luabind::self));
	defDriver.def("GetMemberReference", &game::ValueDriver::GetMemberReference);
	defDriver.def("GetDescriptor", &game::ValueDriver::GetDescriptor);
	defDriverC.scope[defDriver];
	entsMod[defDriverC];
	pragma::LuaCore::define_custom_constructor<game::ValueDriver,
	  +[](ComponentId componentId, const std::string &memberRef, game::ValueDriverDescriptor descriptor, const std::string &self) -> game::ValueDriver { return game::ValueDriver {componentId, memberRef, descriptor, util::uuid_string_to_bytes(self)}; }, ComponentId,
	  const std::string &, game::ValueDriverDescriptor, const std::string &>(GetLuaState());

	auto defIK = pragma::LuaCore::create_entity_component_class<IKComponent, BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled", &IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled", &IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos", &IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos", &IKComponent::GetIKEffectorPos);
	entsMod[defIK];

	auto defOrigin = pragma::LuaCore::create_entity_component_class<OriginComponent, BaseEntityComponent>("OriginComponent");
	defOrigin.add_static_constant("EVENT_ON_ORIGIN_CHANGED", originComponent::EVENT_ON_ORIGIN_CHANGED);
	defOrigin.def("GetOriginPose", &OriginComponent::GetOriginPose);
	defOrigin.def("GetOriginPos", &OriginComponent::GetOriginPos);
	defOrigin.def("GetOriginRot", &OriginComponent::GetOriginRot);
	defOrigin.def("SetOriginPose", &OriginComponent::SetOriginPose);
	defOrigin.def("SetOriginPos", &OriginComponent::SetOriginPos);
	defOrigin.def("SetOriginRot", &OriginComponent::SetOriginRot);
	entsMod[defOrigin];

	auto defConstraint = pragma::LuaCore::create_entity_component_class<ConstraintComponent, BaseEntityComponent>("ConstraintComponent");

	auto defPart = luabind::class_<ConstraintComponent::ConstraintParticipants>("ConstraintParticipants");
	defPart.def_readonly("driver", &ConstraintComponent::ConstraintParticipants::driverC);
	defPart.def_readonly("drivenObject", &ConstraintComponent::ConstraintParticipants::drivenObjectC);
	defPart.def_readonly("driverPropertyIndex", &ConstraintComponent::ConstraintParticipants::driverPropIdx);
	defPart.def_readonly("drivenObjectPropertyIndex", &ConstraintComponent::ConstraintParticipants::drivenObjectPropIdx);
	defConstraint.scope[defPart];

	defConstraint.add_static_constant("EVENT_ON_ORDER_INDEX_CHANGED", constraintComponent::EVENT_ON_ORDER_INDEX_CHANGED);
	defConstraint.add_static_constant("EVENT_APPLY_CONSTRAINT", constraintComponent::EVENT_APPLY_CONSTRAINT);
	defConstraint.def("SetInfluence", &ConstraintComponent::SetInfluence);
	defConstraint.def("GetInfluence", &ConstraintComponent::GetInfluence);
	defConstraint.def("SetDriver", &ConstraintComponent::SetDriver);
	defConstraint.def("GetDriver", +[](ConstraintComponent &constraint) -> EntityUComponentMemberRef & { return const_cast<EntityUComponentMemberRef &>(constraint.GetDriver()); });
	defConstraint.def("SetDrivenObject", &ConstraintComponent::SetDrivenObject);
	defConstraint.def("GetDrivenObject", +[](ConstraintComponent &constraint) -> EntityUComponentMemberRef & { return const_cast<EntityUComponentMemberRef &>(constraint.GetDrivenObject()); });
	defConstraint.def("SetDriverSpace", &ConstraintComponent::SetDriverSpace);
	defConstraint.def("GetDriverSpace", &ConstraintComponent::GetDriverSpace);
	defConstraint.def("SetDrivenObjectSpace", &ConstraintComponent::SetDrivenObjectSpace);
	defConstraint.def("GetDrivenObjectSpace", &ConstraintComponent::GetDrivenObjectSpace);
	defConstraint.def("SetOrderIndex", &ConstraintComponent::SetOrderIndex);
	defConstraint.def("GetOrderIndex", &ConstraintComponent::GetOrderIndex);
	defConstraint.def("GetConstraintParticipants", &ConstraintComponent::GetConstraintParticipants);
	entsMod[defConstraint];

	auto defConstraintManager = pragma::LuaCore::create_entity_component_class<ConstraintManagerComponent, BaseEntityComponent>("ConstraintManagerComponent");
	defConstraintManager.add_static_constant("COORDINATE_SPACE_WORLD", math::to_integral(ConstraintManagerComponent::CoordinateSpace::World));
	defConstraintManager.add_static_constant("COORDINATE_SPACE_LOCAL", math::to_integral(ConstraintManagerComponent::CoordinateSpace::Local));
	defConstraintManager.add_static_constant("COORDINATE_SPACE_OBJECT", math::to_integral(ConstraintManagerComponent::CoordinateSpace::Object));
	defConstraintManager.add_static_constant("EVENT_APPLY_CONSTRAINT", constraintManagerComponent::EVENT_APPLY_CONSTRAINT);
	entsMod[defConstraintManager];

	auto defConstraintSpace = pragma::LuaCore::create_entity_component_class<ConstraintSpaceComponent, BaseEntityComponent>("ConstraintSpaceComponent");
	defConstraintSpace.def("SetAxisEnabled", &ConstraintSpaceComponent::SetAxisEnabled);
	defConstraintSpace.def("IsAxisEnabled", &ConstraintSpaceComponent::IsAxisEnabled);
	defConstraintSpace.def("SetAxisInverted", &ConstraintSpaceComponent::SetAxisInverted);
	defConstraintSpace.def("IsAxisInverted", &ConstraintSpaceComponent::IsAxisInverted);
	defConstraintSpace.def(
	  "ApplyFilter", +[](const ConstraintSpaceComponent &component, const Vector3 &posDriver, const Vector3 &posDriven) -> Vector3 {
		  Vector3 result;
		  component.ApplyFilter(posDriver, posDriven, result);
		  return result;
	  });
	defConstraintSpace.def(
	  "ApplyFilter", +[](const ConstraintSpaceComponent &component, const EulerAngles &angDriver, const EulerAngles &angDriven) -> EulerAngles {
		  EulerAngles result;
		  component.ApplyFilter(angDriver, angDriven, result);
		  return result;
	  });
	entsMod[defConstraintSpace];

	auto defConstraintCopyLocation = pragma::LuaCore::create_entity_component_class<ConstraintCopyLocationComponent, BaseEntityComponent>("ConstraintCopyLocationComponent");
	entsMod[defConstraintCopyLocation];

	auto defConstraintCopyRotation = pragma::LuaCore::create_entity_component_class<ConstraintCopyRotationComponent, BaseEntityComponent>("ConstraintCopyRotationComponent");
	entsMod[defConstraintCopyRotation];

	auto defConstraintCopyScale = pragma::LuaCore::create_entity_component_class<ConstraintCopyScaleComponent, BaseEntityComponent>("ConstraintCopyScaleComponent");
	entsMod[defConstraintCopyScale];

	auto defConstraintLookAtComponent = pragma::LuaCore::create_entity_component_class<ConstraintLookAtComponent, BaseEntityComponent>("ConstraintLookAtComponent");
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_X", math::to_integral(ConstraintLookAtComponent::TrackAxis::X));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_Y", math::to_integral(ConstraintLookAtComponent::TrackAxis::Y));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_Z", math::to_integral(ConstraintLookAtComponent::TrackAxis::Z));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_X", math::to_integral(ConstraintLookAtComponent::TrackAxis::NegX));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_Y", math::to_integral(ConstraintLookAtComponent::TrackAxis::NegY));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_Z", math::to_integral(ConstraintLookAtComponent::TrackAxis::NegZ));
	defConstraintLookAtComponent.def("SetTrackAxis", &ConstraintLookAtComponent::SetTrackAxis);
	defConstraintLookAtComponent.def("GetTrackAxis", &ConstraintLookAtComponent::GetTrackAxis);
	defConstraintLookAtComponent.def("SetUpTarget", &ConstraintLookAtComponent::SetUpTarget);
	defConstraintLookAtComponent.def("GetUpTarget", &ConstraintLookAtComponent::GetUpTarget);
	entsMod[defConstraintLookAtComponent];

	auto defConstraintChildOfComponent = pragma::LuaCore::create_entity_component_class<ConstraintChildOfComponent, BaseEntityComponent>("ConstraintChildOfComponent");
	defConstraintChildOfComponent.def("SetLocationAxisEnabled", &ConstraintChildOfComponent::SetLocationAxisEnabled);
	defConstraintChildOfComponent.def("IsLocationAxisEnabled", &ConstraintChildOfComponent::IsLocationAxisEnabled);
	defConstraintChildOfComponent.def("SetRotationAxisEnabled", &ConstraintChildOfComponent::SetRotationAxisEnabled);
	defConstraintChildOfComponent.def("IsRotationAxisEnabled", &ConstraintChildOfComponent::IsRotationAxisEnabled);
	defConstraintChildOfComponent.def("SetScaleAxisEnabled", &ConstraintChildOfComponent::SetScaleAxisEnabled);
	defConstraintChildOfComponent.def("IsScaleAxisEnabled", &ConstraintChildOfComponent::IsScaleAxisEnabled);
	defConstraintChildOfComponent.def("CalcInversePose", &ConstraintChildOfComponent::CalcInversePose);
	defConstraintChildOfComponent.def("GetConstraint", &ConstraintChildOfComponent::GetConstraint);
	defConstraintChildOfComponent.def("GetDriverPose", &ConstraintChildOfComponent::GetDriverPose);
	defConstraintChildOfComponent.def("GetDrivenPose", &ConstraintChildOfComponent::GetDrivenPose);
	entsMod[defConstraintChildOfComponent];

	auto defConstraintLimitDistance = pragma::LuaCore::create_entity_component_class<ConstraintLimitDistanceComponent, BaseEntityComponent>("ConstraintLimitDistanceComponent");
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_INSIDE", math::to_integral(ConstraintLimitDistanceComponent::ClampRegion::Inside));
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_OUTSIDE", math::to_integral(ConstraintLimitDistanceComponent::ClampRegion::Outside));
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_ON_SURFACE", math::to_integral(ConstraintLimitDistanceComponent::ClampRegion::OnSurface));
	defConstraintLimitDistance.def("SetClampRegion", &ConstraintLimitDistanceComponent::SetClampRegion);
	defConstraintLimitDistance.def("GetClampRegion", &ConstraintLimitDistanceComponent::GetClampRegion);
	defConstraintLimitDistance.def("SetDistance", &ConstraintLimitDistanceComponent::SetDistance);
	defConstraintLimitDistance.def("GetDistance", &ConstraintLimitDistanceComponent::GetDistance);
	entsMod[defConstraintLimitDistance];

	auto defConstraintLimitLocation = pragma::LuaCore::create_entity_component_class<ConstraintLimitLocationComponent, BaseEntityComponent>("ConstraintLimitLocationComponent");
	defConstraintLimitLocation.def("SetMinimum", &ConstraintLimitLocationComponent::SetMinimum);
	defConstraintLimitLocation.def("SetMaximum", &ConstraintLimitLocationComponent::SetMaximum);
	defConstraintLimitLocation.def("GetMinimum", &ConstraintLimitLocationComponent::GetMinimum);
	defConstraintLimitLocation.def("GetMaximum", &ConstraintLimitLocationComponent::GetMaximum);
	defConstraintLimitLocation.def("SetMinimumEnabled", &ConstraintLimitLocationComponent::SetMinimumEnabled);
	defConstraintLimitLocation.def("IsMinimumEnabled", &ConstraintLimitLocationComponent::IsMinimumEnabled);
	defConstraintLimitLocation.def("SetMaximumEnabled", &ConstraintLimitLocationComponent::SetMaximumEnabled);
	defConstraintLimitLocation.def("IsMaximumEnabled", &ConstraintLimitLocationComponent::IsMaximumEnabled);
	entsMod[defConstraintLimitLocation];

	auto defConstraintLimitRotation = pragma::LuaCore::create_entity_component_class<ConstraintLimitRotationComponent, BaseEntityComponent>("ConstraintLimitRotationComponent");
	defConstraintLimitRotation.def("SetLimit", &ConstraintLimitRotationComponent::SetLimit);
	defConstraintLimitRotation.def("GetLimit", &ConstraintLimitRotationComponent::GetLimit);
	defConstraintLimitRotation.def("SetLimitEnabled", &ConstraintLimitRotationComponent::SetLimitEnabled);
	defConstraintLimitRotation.def("IsLimitEnabled", &ConstraintLimitRotationComponent::IsLimitEnabled);
	entsMod[defConstraintLimitRotation];

	auto defConstraintLimitScale = pragma::LuaCore::create_entity_component_class<ConstraintLimitScaleComponent, BaseEntityComponent>("ConstraintLimitScaleComponent");
	defConstraintLimitScale.def("SetMinimum", &ConstraintLimitScaleComponent::SetMinimum);
	defConstraintLimitScale.def("SetMaximum", &ConstraintLimitScaleComponent::SetMaximum);
	defConstraintLimitScale.def("GetMinimum", &ConstraintLimitScaleComponent::GetMinimum);
	defConstraintLimitScale.def("GetMaximum", &ConstraintLimitScaleComponent::GetMaximum);
	defConstraintLimitScale.def("SetMinimumEnabled", &ConstraintLimitScaleComponent::SetMinimumEnabled);
	defConstraintLimitScale.def("IsMinimumEnabled", &ConstraintLimitScaleComponent::IsMinimumEnabled);
	defConstraintLimitScale.def("SetMaximumEnabled", &ConstraintLimitScaleComponent::SetMaximumEnabled);
	defConstraintLimitScale.def("IsMaximumEnabled", &ConstraintLimitScaleComponent::IsMaximumEnabled);
	entsMod[defConstraintLimitScale];

	auto defLifelineLink = pragma::LuaCore::create_entity_component_class<LifelineLinkComponent, BaseEntityComponent>("LifelineLinkComponent");
	entsMod[defLifelineLink];

	auto defLogic = pragma::LuaCore::create_entity_component_class<LogicComponent, BaseEntityComponent>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK", logicComponent::EVENT_ON_TICK);
	entsMod[defLogic];

	auto defUsable = pragma::LuaCore::create_entity_component_class<UsableComponent, BaseEntityComponent>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE", usableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE", usableComponent::EVENT_CAN_USE);
	entsMod[defUsable];

	auto defParent = pragma::LuaCore::create_entity_component_class<ParentComponent, BaseEntityComponent>("ParentComponent");
	defParent.def(
	  "GetChildren", +[](lua::State *l, ParentComponent &parentC) -> luabind::object {
		  auto &children = parentC.GetChildren();
		  auto tChildren = luabind::newtable(l);
		  int32_t i = 1;
		  for(auto &hChild : children) {
			  if(hChild.expired())
				  continue;
			  tChildren[i++] = hChild->GetLuaObject();
		  }
		  return tChildren;
	  });
	defParent.def("GetChildCount", +[](ParentComponent &parentC) -> size_t { return parentC.GetChildren().size(); });
	defParent.def(
	  "GetChild", +[](ParentComponent &parentC, size_t index) -> luabind::object {
		  auto &children = parentC.GetChildren();
		  if(index >= children.size() || children[index].expired())
			  return Lua::nil;
		  return children[index]->GetLuaObject();
	  });
	defParent.add_static_constant("EVENT_ON_CHILD_ADDED", parentComponent::EVENT_ON_CHILD_ADDED);
	defParent.add_static_constant("EVENT_ON_CHILD_REMOVED", parentComponent::EVENT_ON_CHILD_REMOVED);
	entsMod[defParent];

	auto defMap = pragma::LuaCore::create_entity_component_class<MapComponent, BaseEntityComponent>("MapComponent");
	defMap.def("GetMapIndex", &MapComponent::GetMapIndex);
	entsMod[defMap];

	auto defSubmergible = pragma::LuaCore::create_entity_component_class<SubmergibleComponent, BaseEntityComponent>("SubmergibleComponent");
	defSubmergible.def("IsSubmerged", &SubmergibleComponent::IsSubmerged);
	defSubmergible.def("IsFullySubmerged", &SubmergibleComponent::IsFullySubmerged);
	defSubmergible.def("GetSubmergedFraction", &SubmergibleComponent::GetSubmergedFraction);
	defSubmergible.def("IsInWater", &SubmergibleComponent::IsInWater);
	defSubmergible.def("GetSubmergedFractionProperty", &SubmergibleComponent::GetSubmergedFractionProperty);
	defSubmergible.def("GetWaterEntity", static_cast<ecs::BaseEntity *(SubmergibleComponent::*)()>(&SubmergibleComponent::GetWaterEntity));
	defSubmergible.add_static_constant("EVENT_ON_WATER_SUBMERGED", submergibleComponent::EVENT_ON_WATER_SUBMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EMERGED", submergibleComponent::EVENT_ON_WATER_EMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_ENTERED", submergibleComponent::EVENT_ON_WATER_ENTERED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EXITED", submergibleComponent::EVENT_ON_WATER_EXITED);
	entsMod[defSubmergible];

	auto defDamageable = pragma::LuaCore::create_entity_component_class<DamageableComponent, BaseEntityComponent>("DamageableComponent");
	defDamageable.def("TakeDamage", &DamageableComponent::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE", damageableComponent::EVENT_ON_TAKE_DAMAGE);
	entsMod[defDamageable];
}
