/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/lua/lentity_components.hpp"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/sh_lua_component_wrapper.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include "pragma/lua/libraries/lfile.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/policies/property_policy.hpp"
#include "pragma/lua/policies/generic_policy.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/libraries/lutil.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/lentity_components_base_types.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/composite_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/components/ik_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/origin_component.hpp"
#include "pragma/entities/components/intersection_handler_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_space_component.hpp"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_location_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_rotation_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_scale_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_distance_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_location_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_rotation_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_scale_component.hpp"
#include "pragma/entities/components/constraints/constraint_look_at_component.hpp"
#include "pragma/entities/components/constraints/constraint_child_of_component.hpp"
#include "pragma/entities/components/lifeline_link_component.hpp"
#include "pragma/entities/components/meta_rig_component.hpp"
#include "pragma/entities/components/bone_merge_component.hpp"
#include "pragma/entities/components/flex_merge_component.hpp"
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/global_string_converter_t.hpp"
#include "pragma/lua/lua_util_component.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <pragma/physics/movetypes.h>
#include <luabind/copy_policy.hpp>
#include <panima/animation.hpp>
#include <panima/animation_manager.hpp>
#include <panima/channel.hpp>

namespace Lua {
	bool get_bullet_master(BaseEntity &ent);
};
bool Lua::get_bullet_master(BaseEntity &ent)
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
AnimationEvent Lua::get_animation_event(lua_State *l, int32_t tArgs, uint32_t eventId)
{
	Lua::CheckTable(l, tArgs);
	AnimationEvent ev {};
	ev.eventID = static_cast<AnimationEvent::Type>(eventId);
	auto numArgs = Lua::GetObjectLength(l, tArgs);
	for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
		Lua::PushInt(l, i + 1); /* 1 */
		Lua::GetTableValue(l, tArgs);

		auto *arg = Lua::CheckString(l, -1);
		ev.arguments.push_back(arg);

		Lua::Pop(l, 1);
	}
	return ev;
}

static int lua_match_component_member_reference(lua_State *l, int index) { return lua_isstring(l, index) ? 0 : luabind::no_match; }
static pragma::ComponentMemberReference lua_to_component_member_reference(lua_State *l, int index) { return pragma::ComponentMemberReference {Lua::CheckString(l, index)}; }

template<uint32_t N>
using ComponentMemberReferencePolicy = luabind::generic_policy<N, pragma::ComponentMemberReference, &lua_match_component_member_reference, &lua_to_component_member_reference>;

/*template<uint32_t N>
	using UniversalReferencePolicy = luabind::generic_policy<N,util::Uuid,[](lua_State *l,int index) -> int {
		return lua_isstring(l,index) ? 0 : luabind::no_match;
	},[](lua_State *l,int index) -> util::Uuid {
		return util::uuid_string_to_bytes(Lua::CheckString(l,index));
	}>;
*/

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityUComponentRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityUComponentMemberRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::MultiEntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::MultiEntityUComponentRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, ValueDriver);
#endif

enum class BvhIntersectionFlags : uint32_t {
	None = 0u,
	ReturnPrimitives = 1u,
	ReturnMeshes = ReturnPrimitives << 1u,
};
REGISTER_BASIC_BITWISE_OPERATORS(BvhIntersectionFlags)
using IntersectionTestResult = Lua::type<std::pair<bool, Lua::var<std::optional<std::vector<uint64_t>>, std::optional<std::vector<pragma::MeshIntersectionInfo::MeshInfo>>>>>;
static IntersectionTestResult bvh_intersection_test(lua_State *l, const std::function<bool(pragma::IntersectionInfo *)> &fTest, BvhIntersectionFlags flags)
{
	if(!umath::is_flag_set(flags, BvhIntersectionFlags::ReturnPrimitives | BvhIntersectionFlags::ReturnMeshes)) {
		auto res = fTest(nullptr);
		return luabind::object {l, std::pair<bool, std::optional<std::vector<uint64_t>>> {res, {}}};
	}
	if(umath::is_flag_set(flags, BvhIntersectionFlags::ReturnPrimitives)) {
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

template<typename TResult, bool (pragma::MetaRigComponent::*GetValue)(pragma::animation::MetaRigBoneType, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_meta_bone_value(const pragma::MetaRigComponent &metaC, pragma::animation::MetaRigBoneType bone, umath::CoordinateSpace space)
{
	TResult result;
	if(!(metaC.*GetValue)(bone, result, space))
		return {};
	return result;
}
template<typename TResult, bool (pragma::MetaRigComponent::*GetValue)(pragma::animation::MetaRigBoneType, TResult &, umath::CoordinateSpace) const>
std::optional<TResult> get_meta_bone_value_ls(const pragma::MetaRigComponent &metaC, pragma::animation::MetaRigBoneType bone)
{
	return get_meta_bone_value<TResult, GetValue>(metaC, bone, umath::CoordinateSpace::Local);
}

void Game::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	pragma::lua::register_entity_component_classes(GetLuaState(), entsMod);
	Lua::register_gravity_component(entsMod);

	auto classDefEntRef = luabind::class_<pragma::EntityURef>("UniversalEntityReference");
	classDefEntRef.def(luabind::constructor<const BaseEntity &>());
	classDefEntRef.def(luabind::constructor<const std::string &>());
	classDefEntRef.def(luabind::tostring(luabind::self));
	classDefEntRef.def("GetEntity", static_cast<BaseEntity *(pragma::EntityURef::*)(Game &)>(&pragma::EntityURef::GetEntity));
	classDefEntRef.def(
	  "GetUuid", +[](pragma::EntityURef &uref) -> std::optional<Lua::util::Uuid> {
		  auto uuid = uref.GetUuid();
		  return uuid.has_value() ? Lua::util::Uuid {*uuid} : std::optional<Lua::util::Uuid> {};
	  });
	classDefEntRef.def("GetClassOrName", static_cast<std::optional<std::string> (pragma::EntityURef::*)() const>(&pragma::EntityURef::GetClassOrName));
	entsMod[classDefEntRef];
	pragma::lua::define_custom_constructor<pragma::EntityURef, [](const Lua::util::Uuid &uuid) -> pragma::EntityURef { return pragma::EntityURef {uuid.value}; }, const Lua::util::Uuid &>(GetLuaState());

	auto classDefCompRef = luabind::class_<pragma::EntityUComponentRef, pragma::EntityURef>("UniversalComponentReference");
	classDefCompRef.def(luabind::constructor<const std::string &, pragma::ComponentId>());
	classDefCompRef.def(luabind::constructor<const std::string &, const std::string &>());
	classDefCompRef.def(luabind::constructor<const BaseEntity &, pragma::ComponentId>());
	classDefCompRef.def(luabind::tostring(luabind::self));
	classDefCompRef.def("GetComponent", static_cast<pragma::BaseEntityComponent *(pragma::EntityUComponentRef::*)(Game &)>(&pragma::EntityUComponentRef::GetComponent));
	classDefCompRef.def("GetComponentId", &pragma::EntityUComponentMemberRef::GetComponentId);
	classDefCompRef.def(
	  "GetComponentName", +[](const pragma::EntityUComponentRef &ref) -> std::optional<std::string> {
		  auto *name = ref.GetComponentName();
		  if(!name)
			  return {};
		  return *name;
	  });
	entsMod[classDefCompRef];
	pragma::lua::define_custom_constructor<pragma::EntityUComponentRef,
	  [](const Lua::util::Uuid &uuid, pragma::ComponentId componentId) -> pragma::EntityUComponentRef {
		  return pragma::EntityUComponentRef {uuid.value, componentId};
	  },
	  const Lua::util::Uuid &, pragma::ComponentId>(GetLuaState());
	pragma::lua::define_custom_constructor<pragma::EntityUComponentRef,
	  [](const Lua::util::Uuid &uuid, const std::string &componentType) -> pragma::EntityUComponentRef {
		  return pragma::EntityUComponentRef {uuid.value, componentType};
	  },
	  const Lua::util::Uuid &, const std::string &>(GetLuaState());

	auto classDefMemRef = luabind::class_<pragma::EntityUComponentMemberRef, luabind::bases<pragma::EntityUComponentRef, pragma::EntityURef>>("UniversalMemberReference");
	classDefMemRef.def(luabind::constructor<const std::string &, pragma::ComponentId, const std::string &>());
	classDefMemRef.def(luabind::constructor<const std::string &, const std::string &, const std::string &>());
	classDefMemRef.def(luabind::constructor<const BaseEntity &, pragma::ComponentId, const std::string &>());
	classDefMemRef.def(luabind::constructor<const BaseEntity &, const std::string &, const std::string &>());
	classDefMemRef.def(luabind::constructor<const std::string &>());
	classDefMemRef.def(luabind::constructor<>());
	classDefMemRef.def(luabind::tostring(luabind::self));
	classDefMemRef.def("GetMemberInfo", &pragma::EntityUComponentMemberRef::GetMemberInfo);
	classDefMemRef.def(
	  "GetMemberIndex", +[](Game &game, const pragma::EntityUComponentMemberRef &ref) -> std::optional<pragma::ComponentMemberIndex> {
		  if(ref.GetMemberIndex() == pragma::INVALID_COMPONENT_ID)
			  ref.GetMemberInfo(game);
		  auto idx = ref.GetMemberIndex();
		  return (idx != pragma::INVALID_COMPONENT_ID) ? idx : std::optional<pragma::ComponentMemberIndex> {};
	  });
	classDefMemRef.def(
	  "GetMemberName", +[](Game &game, const pragma::EntityUComponentMemberRef &ref) -> std::optional<std::string> {
		  if(ref.GetMemberIndex() == pragma::INVALID_COMPONENT_ID)
			  ref.GetMemberInfo(game);
		  auto name = ref.GetMemberName();
		  return !name.empty() ? name : std::optional<std::string> {};
	  });
	classDefMemRef.def(
	  "GetPath", +[](Game &game, const pragma::EntityUComponentMemberRef &ref) -> std::optional<std::string> {
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
	  "GetValue", +[](lua_State *l, Game &game, const pragma::EntityUComponentMemberRef &ref) {
		  auto *c = ref.GetComponent(game);
		  auto *memberInfo = ref.GetMemberInfo(game);
		  if(!c || !memberInfo)
			  return std::optional<Lua::udm_type> {};
		  return pragma::lua::get_member_value(l, const_cast<pragma::BaseEntityComponent &>(*c), *memberInfo);
	  });
	entsMod[classDefMemRef];
	pragma::lua::define_custom_constructor<pragma::EntityUComponentMemberRef,
	  [](const Lua::util::Uuid &uuid, pragma::ComponentId componentId, const std::string &memberName) -> pragma::EntityUComponentMemberRef {
		  return pragma::EntityUComponentMemberRef {uuid.value, componentId, memberName};
	  },
	  const Lua::util::Uuid &, pragma::ComponentId, const std::string &>(GetLuaState());
	pragma::lua::define_custom_constructor<pragma::EntityUComponentMemberRef,
	  [](const Lua::util::Uuid &uuid, const std::string &componentType, const std::string &memberName) -> pragma::EntityUComponentMemberRef {
		  return pragma::EntityUComponentMemberRef {uuid.value, componentType, memberName};
	  },
	  const Lua::util::Uuid &, const std::string &, const std::string &>(GetLuaState());

	auto classDefMultiEntRef = luabind::class_<pragma::MultiEntityURef>("MultiUniversalEntityReference");
	classDefMultiEntRef.def(luabind::constructor<const BaseEntity &>());
	classDefMultiEntRef.def(luabind::constructor<const std::string &>());
	classDefMultiEntRef.def("FindEntities", &pragma::MultiEntityURef::FindEntities);
	entsMod[classDefMultiEntRef];
	pragma::lua::define_custom_constructor<pragma::MultiEntityURef, [](const Lua::util::Uuid &uuid) -> pragma::MultiEntityURef { return pragma::MultiEntityURef {uuid.value}; }, const Lua::util::Uuid &>(GetLuaState());

	auto defVelocity = pragma::lua::create_entity_component_class<pragma::VelocityComponent, pragma::BaseEntityComponent>("VelocityComponent");
	defVelocity.def("GetVelocity", &pragma::VelocityComponent::GetVelocity, luabind::copy_policy<0> {});
	defVelocity.def("SetVelocity", &pragma::VelocityComponent::SetVelocity);
	defVelocity.def("AddVelocity", &pragma::VelocityComponent::AddVelocity);
	defVelocity.def("GetAngularVelocity", &pragma::VelocityComponent::GetAngularVelocity, luabind::copy_policy<0> {});
	defVelocity.def("SetAngularVelocity", &pragma::VelocityComponent::SetAngularVelocity);
	defVelocity.def("AddAngularVelocity", &pragma::VelocityComponent::AddAngularVelocity);
	defVelocity.def("GetLocalAngularVelocity", &pragma::VelocityComponent::GetLocalAngularVelocity);
	defVelocity.def("SetLocalAngularVelocity", &pragma::VelocityComponent::SetLocalAngularVelocity);
	defVelocity.def("AddLocalAngularVelocity", &pragma::VelocityComponent::AddLocalAngularVelocity);
	defVelocity.def("GetLocalVelocity", &pragma::VelocityComponent::GetLocalVelocity);
	defVelocity.def("SetLocalVelocity", &pragma::VelocityComponent::SetLocalVelocity);
	defVelocity.def("AddLocalVelocity", &pragma::VelocityComponent::AddLocalVelocity);
	defVelocity.def("GetVelocityProperty", &pragma::VelocityComponent::GetVelocityProperty);
	defVelocity.def("GetAngularVelocityProperty", &pragma::VelocityComponent::GetAngularVelocityProperty);
	entsMod[defVelocity];

	auto defMetaRig = pragma::lua::create_entity_component_class<pragma::MetaRigComponent, pragma::BaseEntityComponent>("MetaRigComponent");
	defMetaRig.def("GetBonePose", &get_meta_bone_value<umath::ScaledTransform, &pragma::MetaRigComponent::GetBonePose>);
	defMetaRig.def("GetBonePos", &get_meta_bone_value<Vector3, &pragma::MetaRigComponent::GetBonePos>);
	defMetaRig.def("GetBoneRot", &get_meta_bone_value<Quat, &pragma::MetaRigComponent::GetBoneRot>);
	defMetaRig.def("GetBoneScale", &get_meta_bone_value<Vector3, &pragma::MetaRigComponent::GetBoneScale>);

	defMetaRig.def("GetBonePose", &get_meta_bone_value_ls<umath::ScaledTransform, &pragma::MetaRigComponent::GetBonePose>);
	defMetaRig.def("GetBonePos", &get_meta_bone_value_ls<Vector3, &pragma::MetaRigComponent::GetBonePos>);
	defMetaRig.def("GetBoneRot", &get_meta_bone_value_ls<Quat, &pragma::MetaRigComponent::GetBoneRot>);
	defMetaRig.def("GetBoneScale", &get_meta_bone_value_ls<Vector3, &pragma::MetaRigComponent::GetBoneScale>);

	defMetaRig.def("SetBonePose", &pragma::MetaRigComponent::SetBonePose, luabind::default_parameter_policy<4, umath::CoordinateSpace::Local> {});
	defMetaRig.def("SetBonePos", &pragma::MetaRigComponent::SetBonePos, luabind::default_parameter_policy<4, umath::CoordinateSpace::Local> {});
	defMetaRig.def("SetBoneRot", &pragma::MetaRigComponent::SetBoneRot, luabind::default_parameter_policy<4, umath::CoordinateSpace::Local> {});
	defMetaRig.def("SetBoneScale", &pragma::MetaRigComponent::SetBoneScale, luabind::default_parameter_policy<4, umath::CoordinateSpace::Local> {});
	entsMod[defMetaRig];

	auto defBoneMerge = pragma::lua::create_entity_component_class<pragma::BoneMergeComponent, pragma::BaseEntityComponent>("BoneMergeComponent");
	defBoneMerge.scope[luabind::def("can_merge", &pragma::BoneMergeComponent::can_merge)];
	defBoneMerge.scope[luabind::def("can_merge", &pragma::BoneMergeComponent::can_merge, luabind::default_parameter_policy<3, false> {})];
	defBoneMerge.add_static_constant("EVENT_ON_TARGET_CHANGED", pragma::BoneMergeComponent::EVENT_ON_TARGET_CHANGED);
	defBoneMerge.def("SetTarget", &pragma::BoneMergeComponent::SetTarget);
	defBoneMerge.def("GetTarget", &pragma::BoneMergeComponent::GetTarget);
	entsMod[defBoneMerge];

	auto defFlexMerge = pragma::lua::create_entity_component_class<pragma::FlexMergeComponent, pragma::BaseEntityComponent>("FlexMergeComponent");
	defFlexMerge.scope[luabind::def("can_merge", &pragma::FlexMergeComponent::can_merge)];
	defFlexMerge.add_static_constant("EVENT_ON_TARGET_CHANGED", pragma::FlexMergeComponent::EVENT_ON_TARGET_CHANGED);
	defFlexMerge.def("SetTarget", &pragma::FlexMergeComponent::SetTarget);
	defFlexMerge.def("GetTarget", &pragma::FlexMergeComponent::GetTarget);
	entsMod[defFlexMerge];

	auto defIntersectionHandler = pragma::lua::create_entity_component_class<pragma::IntersectionHandlerComponent, pragma::BaseEntityComponent>("IntersectionHandlerComponent");
	defIntersectionHandler.def(
	  "IntersectionTest2", +[](pragma::IntersectionHandlerComponent &c, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) {
		  auto t = std::chrono::steady_clock::now();
		  c.IntersectionTest(origin, dir, umath::CoordinateSpace::Object, minDist, maxDist);
		  auto dt = std::chrono::steady_clock::now() - t;
		  std::cout << "Internal2: " << (dt.count() / 1'000'000.0) << "ms" << std::endl;
	  });
	defIntersectionHandler.def(
	  "IntersectionTest3", +[](lua_State *l, size_t tStart) {
		  auto t = std::chrono::steady_clock::now();
		  auto dt = std::chrono::steady_clock::now().time_since_epoch().count() - tStart;
		  std::cout << "Lua Overhead: " << (dt / 1'000'000.0) << "ms" << std::endl;
	  });
	defIntersectionHandler.def(
	  "IntersectionTest", +[](pragma::IntersectionHandlerComponent &c, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist) -> std::optional<pragma::HitInfo> { return c.IntersectionTest(origin, dir, umath::CoordinateSpace::Object, minDist, maxDist); });
	// defBvh.def("IntersectionTest", static_cast<std::optional<pragma::bvh::HitInfo> (pragma::IntersectionHandlerComponent::*)(const Vector3 &, const Vector3 &, float, float) const>(&pragma::IntersectionHandlerComponent::IntersectionTest));
	defIntersectionHandler.def("IntersectionTestAabb", static_cast<bool (pragma::IntersectionHandlerComponent::*)(const Vector3 &, const Vector3 &) const>(&pragma::IntersectionHandlerComponent::IntersectionTestAabb));
	defIntersectionHandler.def(
	  "IntersectionTestAabb", +[](lua_State *l, const pragma::IntersectionHandlerComponent &bvhC, const Vector3 &min, const Vector3 &max, BvhIntersectionFlags flags) -> IntersectionTestResult {
		  return bvh_intersection_test(
		    l, [&bvhC, &min, &max](pragma::IntersectionInfo *info) { return info ? bvhC.IntersectionTestAabb(min, max, *info) : bvhC.IntersectionTestAabb(min, max); }, flags);
	  });
	defIntersectionHandler.def("IntersectionTestKDop", static_cast<bool (pragma::IntersectionHandlerComponent::*)(const std::vector<umath::Plane> &) const>(&pragma::IntersectionHandlerComponent::IntersectionTestKDop));
	defIntersectionHandler.def(
	  "IntersectionTestKDop", +[](lua_State *l, const pragma::IntersectionHandlerComponent &bvhC, const std::vector<umath::Plane> &planes, BvhIntersectionFlags flags) -> IntersectionTestResult {
		  return bvh_intersection_test(
		    l, [&bvhC, &planes](pragma::IntersectionInfo *info) { return info ? bvhC.IntersectionTestKDop(planes, *info) : bvhC.IntersectionTestKDop(planes); }, flags);
	  });
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_NONE", umath::to_integral(BvhIntersectionFlags::None));
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_BIT_RETURN_PRIMITIVES", umath::to_integral(BvhIntersectionFlags::ReturnPrimitives));
	defIntersectionHandler.add_static_constant("INTERSECTION_FLAG_BIT_RETURN_MESHES", umath::to_integral(BvhIntersectionFlags::ReturnMeshes));

	auto defBvhHitInfo = luabind::class_<pragma::HitInfo>("HitInfo");
	defBvhHitInfo.def_readonly("mesh", &pragma::HitInfo::mesh);
	defBvhHitInfo.def_readonly("entity", &pragma::HitInfo::entity);
	defBvhHitInfo.property(
	  "entity",
	  +[](lua_State *l, pragma::HitInfo &info) {
		  if(info.entity.expired())
			  Lua::PushNil(l);
		  else
			  info.entity->PushLuaObject(l);
	  },
	  +[](pragma::HitInfo &info, BaseEntity *ent) { info.entity = ent ? ent->GetHandle() : EntityHandle {}; });
	defBvhHitInfo.def_readonly("primitiveIndex", &pragma::HitInfo::primitiveIndex);
	defBvhHitInfo.def_readonly("distance", &pragma::HitInfo::distance);
	defBvhHitInfo.def_readonly("t", &pragma::HitInfo::t);
	defBvhHitInfo.def_readonly("u", &pragma::HitInfo::u);
	defBvhHitInfo.def_readonly("v", &pragma::HitInfo::v);
	defBvhHitInfo.def(
	  "CalcHitNormal", +[](const pragma::HitInfo &hitInfo) -> std::optional<Vector3> {
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
	  "CalcHitUv", +[](const pragma::HitInfo &hitInfo) -> std::optional<Vector2> {
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

	auto defGlobal = pragma::lua::create_entity_component_class<pragma::GlobalNameComponent, pragma::BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName", &pragma::GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName", &pragma::GlobalNameComponent::SetGlobalName);
	entsMod[defGlobal];

	auto defComposite = pragma::lua::create_entity_component_class<pragma::CompositeComponent, pragma::BaseEntityComponent>("CompositeComponent");
	defComposite.def("ClearEntities", &pragma::CompositeComponent::ClearEntities);
	defComposite.def("ClearEntities", &pragma::CompositeComponent::ClearEntities, luabind::default_parameter_policy<2, true> {});
	defComposite.def(
	  "ClearEntities", +[](lua_State *l, pragma::CompositeComponent &hComponent, const std::string &groupName) {
		  auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities();
	  });
	defComposite.def(
	  "ClearEntities", +[](lua_State *l, pragma::CompositeComponent &hComponent, const std::string &groupName, bool safely) {
		  auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities(safely);
	  });
	defComposite.def(
	  "GetEntities", +[](lua_State *l, pragma::CompositeComponent &hComponent) -> luabind::tableT<BaseEntity> {
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
	  "GetEntities", +[](lua_State *l, pragma::CompositeComponent &hComponent, const std::string &groupName) -> luabind::tableT<BaseEntity> {
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
	defComposite.def("GetRootGroup", static_cast<pragma::CompositeGroup &(pragma::CompositeComponent::*)()>(&pragma::CompositeComponent::GetRootCompositeGroup));
	defComposite.def(
	  "AddEntity", +[](lua_State *l, pragma::CompositeComponent &hComponent, BaseEntity &ent) { hComponent.GetRootCompositeGroup().AddEntity(ent); });
	defComposite.def(
	  "AddEntity", +[](lua_State *l, pragma::CompositeComponent &hComponent, BaseEntity &ent, const std::string &groupName) { hComponent.GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(ent); });
	auto defCompositeGroup = luabind::class_<pragma::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity", &pragma::CompositeGroup::AddEntity);
	defCompositeGroup.def(
	  "AddEntity", +[](lua_State *l, pragma::CompositeGroup &hComponent, BaseEntity &ent, const std::string &groupName) { hComponent.AddChildGroup(groupName).AddEntity(ent); });
	defCompositeGroup.def("RemoveEntity", &pragma::CompositeGroup::RemoveEntity);
	defCompositeGroup.def("ClearEntities", &pragma::CompositeGroup::ClearEntities);
	defCompositeGroup.def("ClearEntities", &pragma::CompositeGroup::ClearEntities, luabind::default_parameter_policy<2, true> {});
	defCompositeGroup.def(
	  "ClearEntities", +[](lua_State *l, pragma::CompositeGroup &hComponent, const std::string &groupName) {
		  auto *group = hComponent.FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities();
	  });
	defCompositeGroup.def(
	  "ClearEntities", +[](lua_State *l, pragma::CompositeGroup &hComponent, const std::string &groupName, bool safely) {
		  auto *group = hComponent.FindChildGroup(groupName);
		  if(group)
			  group->ClearEntities(safely);
	  });
	defCompositeGroup.def(
	  "GetEntities", +[](lua_State *l, pragma::CompositeGroup &hComponent) -> luabind::tableT<BaseEntity> {
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
	defCompositeGroup.def("AddChildGroup", &pragma::CompositeGroup::AddChildGroup);
	defCompositeGroup.def(
	  "GetChildGroups", +[](lua_State *l, pragma::CompositeGroup &hComponent, const std::string &name) -> luabind::tableT<pragma::CompositeGroup> {
		  auto t = luabind::newtable(l);
		  int32_t idx = 1;
		  for(auto &childGroup : hComponent.GetChildGroups())
			  t[idx++] = childGroup.get();
		  return t;
	  });
	defComposite.scope[defCompositeGroup];
	entsMod[defComposite];

	auto defAnimated2 = pragma::lua::create_entity_component_class<pragma::PanimaComponent, pragma::BaseEntityComponent>("PanimaComponent");
	defAnimated2.scope[luabind::def("parse_component_channel_path", &pragma::PanimaComponent::ParseComponentChannelPath)];
	defAnimated2.def("ReloadAnimation", static_cast<void (pragma::PanimaComponent::*)()>(&pragma::PanimaComponent::ReloadAnimation));
	defAnimated2.def("SetPlaybackRate", &pragma::PanimaComponent::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate", &pragma::PanimaComponent::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty", &pragma::PanimaComponent::GetPlaybackRateProperty);
	defAnimated2.def("GetCurrentTime", &pragma::PanimaComponent::GetCurrentTime);
	defAnimated2.def("SetCurrentTime", &pragma::PanimaComponent::SetCurrentTime);
	defAnimated2.def("GetCurrentTimeFraction", &pragma::PanimaComponent::GetCurrentTimeFraction);
	defAnimated2.def("SetCurrentTimeFraction", &pragma::PanimaComponent::SetCurrentTimeFraction);
	defAnimated2.def("SetPropertyEnabled", &pragma::PanimaComponent::SetPropertyEnabled);
	defAnimated2.def("IsPropertyEnabled", &pragma::PanimaComponent::IsPropertyEnabled);
	defAnimated2.def("IsPropertyAnimated", &pragma::PanimaComponent::IsPropertyAnimated);
	defAnimated2.def("UpdateAnimationChannelSubmitters", &pragma::PanimaComponent::UpdateAnimationChannelSubmitters);
	defAnimated2.def("ClearAnimationManagers", &pragma::PanimaComponent::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager", &pragma::PanimaComponent::AddAnimationManager);
	defAnimated2.def("AddAnimationManager", &pragma::PanimaComponent::AddAnimationManager, luabind::default_parameter_policy<3, int32_t {0}> {});
	defAnimated2.def("RemoveAnimationManager", static_cast<void (pragma::PanimaComponent::*)(const panima::AnimationManager &)>(&pragma::PanimaComponent::RemoveAnimationManager));
	defAnimated2.def("RemoveAnimationManager", static_cast<void (pragma::PanimaComponent::*)(const std::string_view &)>(&pragma::PanimaComponent::RemoveAnimationManager));
	defAnimated2.def(
	  "GetAnimationManagers", +[](lua_State *l, pragma::PanimaComponent &hComponent) -> luabind::tableT<panima::AnimationManager> {
		  auto t = luabind::newtable(l);
		  auto &animManagers = hComponent.GetAnimationManagers();
		  for(auto &pair : animManagers)
			  t[pair.first] = pair.second;
		  return t;
	  });
	defAnimated2.def(
	  "GetAnimationManager", +[](lua_State *l, pragma::PanimaComponent &hComponent, const std::string &name) { return hComponent.GetAnimationManager(name); });
	defAnimated2.def("PlayAnimation", &pragma::PanimaComponent::PlayAnimation);
	defAnimated2.def("ReloadAnimation", static_cast<void (pragma::PanimaComponent::*)(panima::AnimationManager &)>(&pragma::PanimaComponent::ReloadAnimation));
	defAnimated2.def("AdvanceAnimations", &pragma::PanimaComponent::AdvanceAnimations);
	defAnimated2.def("DebugPrint", static_cast<void (pragma::PanimaComponent::*)()>(&pragma::PanimaComponent::DebugPrint));
	defAnimated2.def(
	  "GetRawAnimatedPropertyValue", +[](lua_State *l, pragma::PanimaComponent &c, panima::AnimationManager &manager, const std::string &propName, udm::Type type) -> Lua::opt<Lua::udm_ng> {
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
	  "GetRawPropertyValue", +[](lua_State *l, pragma::PanimaComponent &c, panima::AnimationManager &manager, const std::string &propName, udm::Type type) -> Lua::opt<Lua::udm_ng> {
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
	defAnimated2.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT", pragma::PanimaComponent::EVENT_HANDLE_ANIMATION_EVENT);
	defAnimated2.add_static_constant("EVENT_ON_PLAY_ANIMATION", pragma::PanimaComponent::EVENT_ON_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_COMPLETE", pragma::PanimaComponent::EVENT_ON_ANIMATION_COMPLETE);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_START", pragma::PanimaComponent::EVENT_ON_ANIMATION_START);
	defAnimated2.add_static_constant("EVENT_MAINTAIN_ANIMATIONS", pragma::PanimaComponent::EVENT_MAINTAIN_ANIMATIONS);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED", pragma::PanimaComponent::EVENT_ON_ANIMATIONS_UPDATED);
	defAnimated2.add_static_constant("EVENT_PLAY_ANIMATION", pragma::PanimaComponent::EVENT_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_TRANSLATE_ANIMATION", pragma::PanimaComponent::EVENT_TRANSLATE_ANIMATION);
	defAnimated2.add_static_constant("EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER", pragma::PanimaComponent::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER);
	entsMod[defAnimated2];

	auto defDriverC = pragma::lua::create_entity_component_class<pragma::AnimationDriverComponent, pragma::BaseEntityComponent>("AnimationDriverComponent");
	defDriverC.def("SetExpression", &pragma::AnimationDriverComponent::SetExpression);
	defDriverC.def("GetExpression", &pragma::AnimationDriverComponent::GetExpression);
	defDriverC.def("AddReference", &pragma::AnimationDriverComponent::AddReference);
	defDriverC.def("GetConstants", &pragma::AnimationDriverComponent::GetConstants);
	defDriverC.def("GetReferences", &pragma::AnimationDriverComponent::GetReferences);
	defDriverC.def("SetDrivenObject", &pragma::AnimationDriverComponent::SetDrivenObject);
	defDriverC.def("GetDrivenObject", &pragma::AnimationDriverComponent::GetDrivenObject);
	defDriverC.def("AddConstant", static_cast<void (pragma::AnimationDriverComponent::*)(const std::string &, const udm::PProperty &)>(&pragma::AnimationDriverComponent::AddConstant));

	auto defDriver = luabind::class_<pragma::ValueDriver>("Driver");
	defDriver.def(luabind::tostring(luabind::self));
	defDriver.def("GetMemberReference", &pragma::ValueDriver::GetMemberReference);
	defDriver.def("GetDescriptor", &pragma::ValueDriver::GetDescriptor);
	defDriverC.scope[defDriver];
	entsMod[defDriverC];
	pragma::lua::define_custom_constructor<pragma::ValueDriver,
	  [](pragma::ComponentId componentId, const std::string &memberRef, pragma::ValueDriverDescriptor descriptor, const std::string &self) -> pragma::ValueDriver {
		  return pragma::ValueDriver {componentId, memberRef, descriptor, util::uuid_string_to_bytes(self)};
	  },
	  pragma::ComponentId, const std::string &, pragma::ValueDriverDescriptor, const std::string &>(GetLuaState());

	auto defIK = pragma::lua::create_entity_component_class<pragma::IKComponent, pragma::BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled", &pragma::IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled", &pragma::IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos", &pragma::IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos", &pragma::IKComponent::GetIKEffectorPos);
	entsMod[defIK];

	auto defOrigin = pragma::lua::create_entity_component_class<pragma::OriginComponent, pragma::BaseEntityComponent>("OriginComponent");
	defOrigin.add_static_constant("EVENT_ON_ORIGIN_CHANGED", pragma::OriginComponent::EVENT_ON_ORIGIN_CHANGED);
	defOrigin.def("GetOriginPose", &pragma::OriginComponent::GetOriginPose);
	defOrigin.def("GetOriginPos", &pragma::OriginComponent::GetOriginPos);
	defOrigin.def("GetOriginRot", &pragma::OriginComponent::GetOriginRot);
	defOrigin.def("SetOriginPose", &pragma::OriginComponent::SetOriginPose);
	defOrigin.def("SetOriginPos", &pragma::OriginComponent::SetOriginPos);
	defOrigin.def("SetOriginRot", &pragma::OriginComponent::SetOriginRot);
	entsMod[defOrigin];

	auto defConstraint = pragma::lua::create_entity_component_class<pragma::ConstraintComponent, pragma::BaseEntityComponent>("ConstraintComponent");

	auto defPart = luabind::class_<pragma::ConstraintComponent::ConstraintParticipants>("ConstraintParticipants");
	defPart.def_readonly("driver", &pragma::ConstraintComponent::ConstraintParticipants::driverC);
	defPart.def_readonly("drivenObject", &pragma::ConstraintComponent::ConstraintParticipants::drivenObjectC);
	defPart.def_readonly("driverPropertyIndex", &pragma::ConstraintComponent::ConstraintParticipants::driverPropIdx);
	defPart.def_readonly("drivenObjectPropertyIndex", &pragma::ConstraintComponent::ConstraintParticipants::drivenObjectPropIdx);
	defConstraint.scope[defPart];

	defConstraint.add_static_constant("EVENT_ON_ORDER_INDEX_CHANGED", pragma::ConstraintComponent::EVENT_ON_ORDER_INDEX_CHANGED);
	defConstraint.add_static_constant("EVENT_APPLY_CONSTRAINT", pragma::ConstraintComponent::EVENT_APPLY_CONSTRAINT);
	defConstraint.def("SetInfluence", &pragma::ConstraintComponent::SetInfluence);
	defConstraint.def("GetInfluence", &pragma::ConstraintComponent::GetInfluence);
	defConstraint.def("SetDriver", &pragma::ConstraintComponent::SetDriver);
	defConstraint.def(
	  "GetDriver", +[](pragma::ConstraintComponent &constraint) -> pragma::EntityUComponentMemberRef & { return const_cast<pragma::EntityUComponentMemberRef &>(constraint.GetDriver()); });
	defConstraint.def("SetDrivenObject", &pragma::ConstraintComponent::SetDrivenObject);
	defConstraint.def(
	  "GetDrivenObject", +[](pragma::ConstraintComponent &constraint) -> pragma::EntityUComponentMemberRef & { return const_cast<pragma::EntityUComponentMemberRef &>(constraint.GetDrivenObject()); });
	defConstraint.def("SetDriverSpace", &pragma::ConstraintComponent::SetDriverSpace);
	defConstraint.def("GetDriverSpace", &pragma::ConstraintComponent::GetDriverSpace);
	defConstraint.def("SetDrivenObjectSpace", &pragma::ConstraintComponent::SetDrivenObjectSpace);
	defConstraint.def("GetDrivenObjectSpace", &pragma::ConstraintComponent::GetDrivenObjectSpace);
	defConstraint.def("SetOrderIndex", &pragma::ConstraintComponent::SetOrderIndex);
	defConstraint.def("GetOrderIndex", &pragma::ConstraintComponent::GetOrderIndex);
	defConstraint.def("GetConstraintParticipants", &pragma::ConstraintComponent::GetConstraintParticipants);
	entsMod[defConstraint];

	auto defConstraintManager = pragma::lua::create_entity_component_class<pragma::ConstraintManagerComponent, pragma::BaseEntityComponent>("ConstraintManagerComponent");
	defConstraintManager.add_static_constant("COORDINATE_SPACE_WORLD", umath::to_integral(pragma::ConstraintManagerComponent::CoordinateSpace::World));
	defConstraintManager.add_static_constant("COORDINATE_SPACE_LOCAL", umath::to_integral(pragma::ConstraintManagerComponent::CoordinateSpace::Local));
	defConstraintManager.add_static_constant("COORDINATE_SPACE_OBJECT", umath::to_integral(pragma::ConstraintManagerComponent::CoordinateSpace::Object));
	defConstraintManager.add_static_constant("EVENT_APPLY_CONSTRAINT", pragma::ConstraintManagerComponent::EVENT_APPLY_CONSTRAINT);
	entsMod[defConstraintManager];

	auto defConstraintSpace = pragma::lua::create_entity_component_class<pragma::ConstraintSpaceComponent, pragma::BaseEntityComponent>("ConstraintSpaceComponent");
	defConstraintSpace.def("SetAxisEnabled", &pragma::ConstraintSpaceComponent::SetAxisEnabled);
	defConstraintSpace.def("IsAxisEnabled", &pragma::ConstraintSpaceComponent::IsAxisEnabled);
	defConstraintSpace.def("SetAxisInverted", &pragma::ConstraintSpaceComponent::SetAxisInverted);
	defConstraintSpace.def("IsAxisInverted", &pragma::ConstraintSpaceComponent::IsAxisInverted);
	defConstraintSpace.def(
	  "ApplyFilter", +[](const pragma::ConstraintSpaceComponent &component, const Vector3 &posDriver, const Vector3 &posDriven) -> Vector3 {
		  Vector3 result;
		  component.ApplyFilter(posDriver, posDriven, result);
		  return result;
	  });
	defConstraintSpace.def(
	  "ApplyFilter", +[](const pragma::ConstraintSpaceComponent &component, const EulerAngles &angDriver, const EulerAngles &angDriven) -> EulerAngles {
		  EulerAngles result;
		  component.ApplyFilter(angDriver, angDriven, result);
		  return result;
	  });
	entsMod[defConstraintSpace];

	auto defConstraintCopyLocation = pragma::lua::create_entity_component_class<pragma::ConstraintCopyLocationComponent, pragma::BaseEntityComponent>("ConstraintCopyLocationComponent");
	entsMod[defConstraintCopyLocation];

	auto defConstraintCopyRotation = pragma::lua::create_entity_component_class<pragma::ConstraintCopyRotationComponent, pragma::BaseEntityComponent>("ConstraintCopyRotationComponent");
	entsMod[defConstraintCopyRotation];

	auto defConstraintCopyScale = pragma::lua::create_entity_component_class<pragma::ConstraintCopyScaleComponent, pragma::BaseEntityComponent>("ConstraintCopyScaleComponent");
	entsMod[defConstraintCopyScale];

	auto defConstraintLookAtComponent = pragma::lua::create_entity_component_class<pragma::ConstraintLookAtComponent, pragma::BaseEntityComponent>("ConstraintLookAtComponent");
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_X", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::X));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_Y", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::Y));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_Z", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::Z));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_X", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::NegX));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_Y", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::NegY));
	defConstraintLookAtComponent.add_static_constant("TRACK_AXIS_NEG_Z", umath::to_integral(pragma::ConstraintLookAtComponent::TrackAxis::NegZ));
	defConstraintLookAtComponent.def("SetTrackAxis", &pragma::ConstraintLookAtComponent::SetTrackAxis);
	defConstraintLookAtComponent.def("GetTrackAxis", &pragma::ConstraintLookAtComponent::GetTrackAxis);
	defConstraintLookAtComponent.def("SetUpTarget", &pragma::ConstraintLookAtComponent::SetUpTarget);
	defConstraintLookAtComponent.def("GetUpTarget", &pragma::ConstraintLookAtComponent::GetUpTarget);
	entsMod[defConstraintLookAtComponent];

	auto defConstraintChildOfComponent = pragma::lua::create_entity_component_class<pragma::ConstraintChildOfComponent, pragma::BaseEntityComponent>("ConstraintChildOfComponent");
	defConstraintChildOfComponent.def("SetLocationAxisEnabled", &pragma::ConstraintChildOfComponent::SetLocationAxisEnabled);
	defConstraintChildOfComponent.def("IsLocationAxisEnabled", &pragma::ConstraintChildOfComponent::IsLocationAxisEnabled);
	defConstraintChildOfComponent.def("SetRotationAxisEnabled", &pragma::ConstraintChildOfComponent::SetRotationAxisEnabled);
	defConstraintChildOfComponent.def("IsRotationAxisEnabled", &pragma::ConstraintChildOfComponent::IsRotationAxisEnabled);
	defConstraintChildOfComponent.def("SetScaleAxisEnabled", &pragma::ConstraintChildOfComponent::SetScaleAxisEnabled);
	defConstraintChildOfComponent.def("IsScaleAxisEnabled", &pragma::ConstraintChildOfComponent::IsScaleAxisEnabled);
	defConstraintChildOfComponent.def("CalcInversePose", &pragma::ConstraintChildOfComponent::CalcInversePose);
	defConstraintChildOfComponent.def("GetConstraint", &pragma::ConstraintChildOfComponent::GetConstraint);
	defConstraintChildOfComponent.def("GetDriverPose", &pragma::ConstraintChildOfComponent::GetDriverPose);
	defConstraintChildOfComponent.def("GetDrivenPose", &pragma::ConstraintChildOfComponent::GetDrivenPose);
	entsMod[defConstraintChildOfComponent];

	auto defConstraintLimitDistance = pragma::lua::create_entity_component_class<pragma::ConstraintLimitDistanceComponent, pragma::BaseEntityComponent>("ConstraintLimitDistanceComponent");
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_INSIDE", umath::to_integral(pragma::ConstraintLimitDistanceComponent::ClampRegion::Inside));
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_OUTSIDE", umath::to_integral(pragma::ConstraintLimitDistanceComponent::ClampRegion::Outside));
	defConstraintLimitDistance.add_static_constant("CLAMP_REGION_ON_SURFACE", umath::to_integral(pragma::ConstraintLimitDistanceComponent::ClampRegion::OnSurface));
	defConstraintLimitDistance.def("SetClampRegion", &pragma::ConstraintLimitDistanceComponent::SetClampRegion);
	defConstraintLimitDistance.def("GetClampRegion", &pragma::ConstraintLimitDistanceComponent::GetClampRegion);
	defConstraintLimitDistance.def("SetDistance", &pragma::ConstraintLimitDistanceComponent::SetDistance);
	defConstraintLimitDistance.def("GetDistance", &pragma::ConstraintLimitDistanceComponent::GetDistance);
	entsMod[defConstraintLimitDistance];

	auto defConstraintLimitLocation = pragma::lua::create_entity_component_class<pragma::ConstraintLimitLocationComponent, pragma::BaseEntityComponent>("ConstraintLimitLocationComponent");
	defConstraintLimitLocation.def("SetMinimum", &pragma::ConstraintLimitLocationComponent::SetMinimum);
	defConstraintLimitLocation.def("SetMaximum", &pragma::ConstraintLimitLocationComponent::SetMaximum);
	defConstraintLimitLocation.def("GetMinimum", &pragma::ConstraintLimitLocationComponent::GetMinimum);
	defConstraintLimitLocation.def("GetMaximum", &pragma::ConstraintLimitLocationComponent::GetMaximum);
	defConstraintLimitLocation.def("SetMinimumEnabled", &pragma::ConstraintLimitLocationComponent::SetMinimumEnabled);
	defConstraintLimitLocation.def("IsMinimumEnabled", &pragma::ConstraintLimitLocationComponent::IsMinimumEnabled);
	defConstraintLimitLocation.def("SetMaximumEnabled", &pragma::ConstraintLimitLocationComponent::SetMaximumEnabled);
	defConstraintLimitLocation.def("IsMaximumEnabled", &pragma::ConstraintLimitLocationComponent::IsMaximumEnabled);
	entsMod[defConstraintLimitLocation];

	auto defConstraintLimitRotation = pragma::lua::create_entity_component_class<pragma::ConstraintLimitRotationComponent, pragma::BaseEntityComponent>("ConstraintLimitRotationComponent");
	defConstraintLimitRotation.def("SetLimit", &pragma::ConstraintLimitRotationComponent::SetLimit);
	defConstraintLimitRotation.def("GetLimit", &pragma::ConstraintLimitRotationComponent::GetLimit);
	defConstraintLimitRotation.def("SetLimitEnabled", &pragma::ConstraintLimitRotationComponent::SetLimitEnabled);
	defConstraintLimitRotation.def("IsLimitEnabled", &pragma::ConstraintLimitRotationComponent::IsLimitEnabled);
	entsMod[defConstraintLimitRotation];

	auto defConstraintLimitScale = pragma::lua::create_entity_component_class<pragma::ConstraintLimitScaleComponent, pragma::BaseEntityComponent>("ConstraintLimitScaleComponent");
	defConstraintLimitScale.def("SetMinimum", &pragma::ConstraintLimitScaleComponent::SetMinimum);
	defConstraintLimitScale.def("SetMaximum", &pragma::ConstraintLimitScaleComponent::SetMaximum);
	defConstraintLimitScale.def("GetMinimum", &pragma::ConstraintLimitScaleComponent::GetMinimum);
	defConstraintLimitScale.def("GetMaximum", &pragma::ConstraintLimitScaleComponent::GetMaximum);
	defConstraintLimitScale.def("SetMinimumEnabled", &pragma::ConstraintLimitScaleComponent::SetMinimumEnabled);
	defConstraintLimitScale.def("IsMinimumEnabled", &pragma::ConstraintLimitScaleComponent::IsMinimumEnabled);
	defConstraintLimitScale.def("SetMaximumEnabled", &pragma::ConstraintLimitScaleComponent::SetMaximumEnabled);
	defConstraintLimitScale.def("IsMaximumEnabled", &pragma::ConstraintLimitScaleComponent::IsMaximumEnabled);
	entsMod[defConstraintLimitScale];

	auto defLifelineLink = pragma::lua::create_entity_component_class<pragma::LifelineLinkComponent, pragma::BaseEntityComponent>("LifelineLinkComponent");
	entsMod[defLifelineLink];

	auto defLogic = pragma::lua::create_entity_component_class<pragma::LogicComponent, pragma::BaseEntityComponent>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK", pragma::LogicComponent::EVENT_ON_TICK);
	entsMod[defLogic];

	auto defUsable = pragma::lua::create_entity_component_class<pragma::UsableComponent, pragma::BaseEntityComponent>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE", pragma::UsableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE", pragma::UsableComponent::EVENT_CAN_USE);
	entsMod[defUsable];

	auto defMap = pragma::lua::create_entity_component_class<pragma::MapComponent, pragma::BaseEntityComponent>("MapComponent");
	defMap.def("GetMapIndex", &pragma::MapComponent::GetMapIndex);
	entsMod[defMap];

	auto defSubmergible = pragma::lua::create_entity_component_class<pragma::SubmergibleComponent, pragma::BaseEntityComponent>("SubmergibleComponent");
	defSubmergible.def("IsSubmerged", &pragma::SubmergibleComponent::IsSubmerged);
	defSubmergible.def("IsFullySubmerged", &pragma::SubmergibleComponent::IsFullySubmerged);
	defSubmergible.def("GetSubmergedFraction", &pragma::SubmergibleComponent::GetSubmergedFraction);
	defSubmergible.def("IsInWater", &pragma::SubmergibleComponent::IsInWater);
	defSubmergible.def("GetSubmergedFractionProperty", &pragma::SubmergibleComponent::GetSubmergedFractionProperty);
	defSubmergible.def("GetWaterEntity", static_cast<BaseEntity *(pragma::SubmergibleComponent::*)()>(&pragma::SubmergibleComponent::GetWaterEntity));
	defSubmergible.add_static_constant("EVENT_ON_WATER_SUBMERGED", pragma::SubmergibleComponent::EVENT_ON_WATER_SUBMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EMERGED", pragma::SubmergibleComponent::EVENT_ON_WATER_EMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_ENTERED", pragma::SubmergibleComponent::EVENT_ON_WATER_ENTERED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EXITED", pragma::SubmergibleComponent::EVENT_ON_WATER_EXITED);
	entsMod[defSubmergible];

	auto defDamageable = pragma::lua::create_entity_component_class<pragma::DamageableComponent, pragma::BaseEntityComponent>("DamageableComponent");
	defDamageable.def("TakeDamage", &pragma::DamageableComponent::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE", pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE);
	entsMod[defDamageable];
}
