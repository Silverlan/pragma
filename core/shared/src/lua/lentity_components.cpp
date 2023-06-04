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
#include "pragma/lua/lentity_components_base_types.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/composite_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/components/ik_component.hpp"
#include "pragma/entities/components/ik_solver_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/ik_solver/rig_config.hpp"
#include "pragma/entities/components/origin_component.hpp"
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
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
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

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityUComponentRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::EntityUComponentMemberRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::MultiEntityURef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, pragma::MultiEntityUComponentRef);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, ValueDriver);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfig);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigBone);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, pragma::ik::RigConfigConstraint);

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
	defAnimated2.def("ClearAnimationManagers", &pragma::PanimaComponent::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager", &pragma::PanimaComponent::AddAnimationManager);
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

	auto defRigConfig = luabind::class_<pragma::ik::RigConfig>("RigConfig");
	defRigConfig.def(luabind::constructor<>());
	defRigConfig.def(luabind::tostring(luabind::self));
	defRigConfig.scope[luabind::def("load", &pragma::ik::RigConfig::load)];
	defRigConfig.scope[luabind::def("load_from_udm_data", &pragma::ik::RigConfig::load_from_udm_data)];
	defRigConfig.scope[luabind::def("get_supported_extensions", &pragma::ik::RigConfig::get_supported_extensions)];
	defRigConfig.def("DebugPrint", &pragma::ik::RigConfig::DebugPrint);
	defRigConfig.def("ToUdmData", &pragma::ik::RigConfig::ToUdmData);
	defRigConfig.def("AddBone", &pragma::ik::RigConfig::AddBone);
	defRigConfig.def("GetBones", &pragma::ik::RigConfig::GetBones);
	defRigConfig.def("FindBone", &pragma::ik::RigConfig::FindBone);
	defRigConfig.def("GetConstraints", &pragma::ik::RigConfig::GetConstraints);
	defRigConfig.def("GetControls", &pragma::ik::RigConfig::GetControls);
	defRigConfig.def("RemoveBone", static_cast<void (pragma::ik::RigConfig::*)(const std::string &)>(&pragma::ik::RigConfig::RemoveBone));
	defRigConfig.def("RemoveControl", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigControl &)>(&pragma::ik::RigConfig::RemoveControl));
	defRigConfig.def("RemoveControl", static_cast<void (pragma::ik::RigConfig::*)(const std::string &)>(&pragma::ik::RigConfig::RemoveControl));
	defRigConfig.def("RemoveConstraint", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigConstraint &)>(&pragma::ik::RigConfig::RemoveConstraint));
	defRigConfig.def("RemoveBone", static_cast<void (pragma::ik::RigConfig::*)(const pragma::ik::RigConfigBone &)>(&pragma::ik::RigConfig::RemoveBone));
	defRigConfig.def("HasBone", &pragma::ik::RigConfig::HasBone);
	defRigConfig.def("IsBoneLocked", &pragma::ik::RigConfig::IsBoneLocked);
	defRigConfig.def("SetBoneLocked", &pragma::ik::RigConfig::SetBoneLocked);
	defRigConfig.def("HasControl", &pragma::ik::RigConfig::HasControl);
	defRigConfig.def("AddControl", &pragma::ik::RigConfig::AddControl);
	defRigConfig.def("RemoveConstraints", static_cast<void (pragma::ik::RigConfig::*)(const std::string &, const std::string &)>(&pragma::ik::RigConfig::RemoveConstraints));
	defRigConfig.def("RemoveConstraints", static_cast<void (pragma::ik::RigConfig::*)(const std::string &)>(&pragma::ik::RigConfig::RemoveConstraints));
	defRigConfig.def("AddFixedConstraint", &pragma::ik::RigConfig::AddFixedConstraint);
	defRigConfig.def("AddHingeConstraint", &pragma::ik::RigConfig::AddHingeConstraint);
	defRigConfig.def("AddBallSocketConstraint", &pragma::ik::RigConfig::AddBallSocketConstraint);
	defRigConfig.def("AddBallSocketConstraint", &pragma::ik::RigConfig::AddBallSocketConstraint, luabind::default_parameter_policy<6, pragma::SignedAxis::Z> {});
	defRigConfig.def(
	  "Save", +[](lua_State *l, pragma::ik::RigConfig &rigConfig, const std::string &fileName) -> std::pair<bool, std::optional<std::string>> {
		  auto fname = fileName;
		  if(Lua::file::validate_write_operation(l, fname) == false)
			  return std::pair<bool, std::optional<std::string>> {false, "Invalid write location!"};
		  auto res = rigConfig.Save(fname);
		  if(!res)
			  return std::pair<bool, std::optional<std::string>> {false, "Unknown error"};
		  return std::pair<bool, std::optional<std::string>> {true, {}};
	  });

	auto defRigBone = luabind::class_<pragma::ik::RigConfigBone>("Bone");
	defRigBone.def(luabind::tostring(luabind::self));
	defRigBone.def_readwrite("locked", &pragma::ik::RigConfigBone::locked);
	defRigBone.def_readwrite("name", &pragma::ik::RigConfigBone::name);
	defRigConfig.scope[defRigBone];

	auto defRigControl = luabind::class_<pragma::ik::RigConfigControl>("Control");
	defRigControl.def(luabind::tostring(luabind::self));
	defRigControl.add_static_constant("TYPE_DRAG", umath::to_integral(pragma::ik::RigConfigControl::Type::Drag));
	defRigControl.add_static_constant("TYPE_STATE", umath::to_integral(pragma::ik::RigConfigControl::Type::State));
	defRigControl.def_readwrite("bone", &pragma::ik::RigConfigControl::bone);
	defRigControl.def_readwrite("type", &pragma::ik::RigConfigControl::type);
	defRigControl.def_readwrite("maxForce", &pragma::ik::RigConfigControl::maxForce);
	defRigControl.def_readwrite("rigidity", &pragma::ik::RigConfigControl::rigidity);
	defRigConfig.scope[defRigControl];
	defRigConfig.scope[defRigBone];

	auto defRigConstraint = luabind::class_<pragma::ik::RigConfigConstraint>("Constraint");
	defRigConstraint.def(luabind::tostring(luabind::self));
	defRigConstraint.add_static_constant("TYPE_FIXED", umath::to_integral(pragma::ik::RigConfigConstraint::Type::Fixed));
	defRigConstraint.add_static_constant("TYPE_HINGE", umath::to_integral(pragma::ik::RigConfigConstraint::Type::Hinge));
	defRigConstraint.add_static_constant("TYPE_BALL_SOCKET", umath::to_integral(pragma::ik::RigConfigConstraint::Type::BallSocket));
	defRigConstraint.def_readwrite("bone0", &pragma::ik::RigConfigConstraint::bone0);
	defRigConstraint.def_readwrite("bone1", &pragma::ik::RigConfigConstraint::bone1);
	defRigConstraint.def_readwrite("type", &pragma::ik::RigConfigConstraint::type);
	defRigConstraint.def_readwrite("minLimits", &pragma::ik::RigConfigConstraint::minLimits);
	defRigConstraint.def_readwrite("maxLimits", &pragma::ik::RigConfigConstraint::maxLimits);
	defRigConstraint.def_readwrite("axis", &pragma::ik::RigConfigConstraint::axis);
	defRigConfig.scope[defRigConstraint];

	auto defIkSolver = pragma::lua::create_entity_component_class<pragma::IkSolverComponent, pragma::BaseEntityComponent>("IkSolverComponent");
	defIkSolver.add_static_constant("EVENT_INITIALIZE_SOLVER", pragma::IkSolverComponent::EVENT_INITIALIZE_SOLVER);
	defIkSolver.add_static_constant("EVENT_UPDATE_IK", pragma::IkSolverComponent::EVENT_UPDATE_IK);
	defIkSolver.scope[luabind::def("find_forward_axis", &pragma::IkSolverComponent::FindTwistAxis)];
	defIkSolver.scope[luabind::def("get_control_bone_name", &pragma::IkSolverComponent::GetControlBoneName)];
	defIkSolver.def("SetIkRigFile", &pragma::IkSolverComponent::SetIkRigFile);
	defIkSolver.def("GetIkRigFile", &pragma::IkSolverComponent::GetIkRigFile);
	defIkSolver.def("AddSkeletalBone", &pragma::IkSolverComponent::AddSkeletalBone);
	defIkSolver.def("SetBoneLocked", &pragma::IkSolverComponent::SetBoneLocked);
	defIkSolver.def("GetBone", &pragma::IkSolverComponent::GetBone);
	defIkSolver.def("GetBoneCount", &pragma::IkSolverComponent::GetBoneCount);
	defIkSolver.def("GetControl", &pragma::IkSolverComponent::GetControl);
	defIkSolver.def("AddDragControl", &pragma::IkSolverComponent::AddDragControl);
	defIkSolver.def("AddStateControl", &pragma::IkSolverComponent::AddStateControl);
	defIkSolver.def("AddFixedConstraint", &pragma::IkSolverComponent::AddFixedConstraint);
	defIkSolver.def("AddHingeConstraint", &pragma::IkSolverComponent::AddHingeConstraint);
	defIkSolver.def("AddBallSocketConstraint", &pragma::IkSolverComponent::AddBallSocketConstraint);
	defIkSolver.def("AddIkSolverByRig", &pragma::IkSolverComponent::AddIkSolverByRig);
	defIkSolver.def("AddIkSolverByChain", &pragma::IkSolverComponent::AddIkSolverByChain);
	defIkSolver.def("GetIkRig", &pragma::IkSolverComponent::GetIkRig);
	defIkSolver.def("GetIkBoneId", &pragma::IkSolverComponent::GetIkBoneId);
	defIkSolver.def("GetControlBoneId", &pragma::IkSolverComponent::GetControlBoneId);
	defIkSolver.def("GetSkeletalBoneId", &pragma::IkSolverComponent::GetSkeletalBoneId);
	defIkSolver.def("Solve", &pragma::IkSolverComponent::Solve);
	defIkSolver.def("ResetIkRig", &pragma::IkSolverComponent::ResetIkRig);
	defIkSolver.def("GetIkSolver", &pragma::IkSolverComponent::GetIkSolver);
	defIkSolver.scope[defRigConfig];
	entsMod[defIkSolver];

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
	entsMod[defConstraintLookAtComponent];

	auto defConstraintChildOfComponent = pragma::lua::create_entity_component_class<pragma::ConstraintChildOfComponent, pragma::BaseEntityComponent>("ConstraintChildOfComponent");
	defConstraintChildOfComponent.def("SetLocationAxisEnabled", &pragma::ConstraintChildOfComponent::SetLocationAxisEnabled);
	defConstraintChildOfComponent.def("IsLocationAxisEnabled", &pragma::ConstraintChildOfComponent::IsLocationAxisEnabled);
	defConstraintChildOfComponent.def("SetRotationAxisEnabled", &pragma::ConstraintChildOfComponent::SetRotationAxisEnabled);
	defConstraintChildOfComponent.def("IsRotationAxisEnabled", &pragma::ConstraintChildOfComponent::IsRotationAxisEnabled);
	defConstraintChildOfComponent.def("SetScaleAxisEnabled", &pragma::ConstraintChildOfComponent::SetScaleAxisEnabled);
	defConstraintChildOfComponent.def("IsScaleAxisEnabled", &pragma::ConstraintChildOfComponent::IsScaleAxisEnabled);
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
