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
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/policies/property_policy.hpp"
#include "pragma/lua/policies/generic_policy.hpp"
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/lua/lentity_components_base_types.hpp"
#include "pragma/entities/components/animated_2_component.hpp"
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
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include <pragma/physics/movetypes.h>
#include <luabind/copy_policy.hpp>
#include <panima/animation.hpp>
#include <panima/animation_manager.hpp>
#include <panima/channel.hpp>
#pragma optimize("",off)
namespace Lua {bool get_bullet_master(BaseEntity &ent);};
bool Lua::get_bullet_master(BaseEntity &ent)
{
	auto bMaster = true;
	if(ent.IsWeapon())
	{
		auto &wepComponent = *ent.GetWeaponComponent();
		auto *ownerComponent = wepComponent.GetOwnerComponent();
		auto *owner = (ownerComponent != nullptr) ? ownerComponent->GetOwner() : nullptr;
		if(owner != nullptr && owner->IsPlayer())
		{
			auto plComponent = owner->GetPlayerComponent();
			if(ent.GetNetworkState()->IsServer() || plComponent.expired() || !plComponent->IsLocalPlayer())
				bMaster = false; // Assume that shot originated from other client
		}
	}
	return bMaster;
}
AnimationEvent Lua::get_animation_event(lua_State *l,int32_t tArgs,uint32_t eventId)
{
	Lua::CheckTable(l,tArgs);
	AnimationEvent ev {};
	ev.eventID = static_cast<AnimationEvent::Type>(eventId);
	auto numArgs = Lua::GetObjectLength(l,tArgs);
	for(auto i=decltype(numArgs){0};i<numArgs;++i)
	{
		Lua::PushInt(l,i +1); /* 1 */
		Lua::GetTableValue(l,tArgs);

		auto *arg = Lua::CheckString(l,-1);
		ev.arguments.push_back(arg);

		Lua::Pop(l,1);
	}
	return ev;
}

template<typename TMemberId> requires(std::is_same_v<TMemberId,pragma::ComponentMemberIndex> || std::is_same_v<TMemberId,const std::string&>)
	static void add_driver(
	pragma::AnimationDriverComponent &hComponent,pragma::ComponentId componentId,TMemberId memberIdx,pragma::ValueDriverDescriptor descriptor
)
{
	hComponent.AddDriver(componentId,memberIdx,std::move(descriptor));
}

template<uint32_t N>
	using ComponentMemberReferencePolicy = luabind::generic_policy<N,pragma::ComponentMemberReference,[](lua_State *l,int index) -> int {
		return lua_isstring(l,index) ? 0 : luabind::no_match;
	},[](lua_State *l,int index) -> pragma::ComponentMemberReference {
		return pragma::ComponentMemberReference{Lua::CheckString(l,index)};
	}>;

template<uint32_t N>
	using UuidPolicy = luabind::generic_policy<N,util::Uuid,[](lua_State *l,int index) -> int {
		return lua_isstring(l,index) ? 0 : luabind::no_match;
	},[](lua_State *l,int index) -> util::Uuid {
		return util::uuid_string_to_bytes(Lua::CheckString(l,index));
	}>;

void Game::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	pragma::lua::register_entity_component_classes(entsMod);
	Lua::register_gravity_component(entsMod);

	auto classDefEntRef = luabind::class_<pragma::EntityUuidRef>("UniversalEntityReference");
	classDefEntRef.def(luabind::constructor<const BaseEntity&>());
	classDefEntRef.def(luabind::constructor<util::Uuid>(),UuidPolicy<2>{});
	classDefEntRef.def("GetEntity",static_cast<BaseEntity*(pragma::EntityUuidRef::*)(Game&)>(&pragma::EntityUuidRef::GetEntity));
	entsMod[classDefEntRef];

	auto classDefCompRef = luabind::class_<pragma::EntityUuidComponentRef,pragma::EntityUuidRef>("UniversalComponentReference");
	classDefCompRef.def(luabind::constructor<util::Uuid,pragma::ComponentId>(),UuidPolicy<2>{});
	classDefCompRef.def(luabind::constructor<util::Uuid,const std::string&>(),UuidPolicy<2>{});
	classDefCompRef.def(luabind::constructor<const BaseEntity&,pragma::ComponentId>());
	classDefCompRef.def("GetComponent",static_cast<pragma::BaseEntityComponent*(pragma::EntityUuidComponentRef::*)(Game&)>(&pragma::EntityUuidComponentRef::GetComponent));
	entsMod[classDefCompRef];

	auto classDefMemRef = luabind::class_<pragma::EntityUuidComponentMemberRef,luabind::bases<pragma::EntityUuidComponentRef,pragma::EntityUuidRef>>("UniversalMemberReference");
	classDefMemRef.def(luabind::constructor<util::Uuid,pragma::ComponentId,const std::string&>(),UuidPolicy<2>{});
	classDefMemRef.def(luabind::constructor<util::Uuid,const std::string&,const std::string&>(),UuidPolicy<2>{});
	classDefMemRef.def(luabind::constructor<const BaseEntity&,pragma::ComponentId,const std::string&>());
	classDefMemRef.def(luabind::constructor<const BaseEntity&,const std::string&,const std::string&>());
	classDefMemRef.def("GetMemberInfo",&pragma::EntityUuidComponentMemberRef::GetMemberInfo);
	entsMod[classDefMemRef];

	auto defVelocity = luabind::class_<pragma::VelocityComponent,pragma::BaseEntityComponent>("VelocityComponent");
	defVelocity.def("GetVelocity",&pragma::VelocityComponent::GetVelocity,luabind::copy_policy<0>{});
	defVelocity.def("SetVelocity",&pragma::VelocityComponent::SetVelocity);
	defVelocity.def("AddVelocity",&pragma::VelocityComponent::AddVelocity);
	defVelocity.def("GetAngularVelocity",&pragma::VelocityComponent::GetAngularVelocity,luabind::copy_policy<0>{});
	defVelocity.def("SetAngularVelocity",&pragma::VelocityComponent::SetAngularVelocity);
	defVelocity.def("AddAngularVelocity",&pragma::VelocityComponent::AddAngularVelocity);
	defVelocity.def("GetLocalAngularVelocity",&pragma::VelocityComponent::GetLocalAngularVelocity);
	defVelocity.def("SetLocalAngularVelocity",&pragma::VelocityComponent::SetLocalAngularVelocity);
	defVelocity.def("AddLocalAngularVelocity",&pragma::VelocityComponent::AddLocalAngularVelocity);
	defVelocity.def("GetLocalVelocity",&pragma::VelocityComponent::GetLocalVelocity);
	defVelocity.def("SetLocalVelocity",&pragma::VelocityComponent::SetLocalVelocity);
	defVelocity.def("AddLocalVelocity",&pragma::VelocityComponent::AddLocalVelocity);
	defVelocity.def("GetVelocityProperty",&pragma::VelocityComponent::GetVelocityProperty);
	defVelocity.def("GetAngularVelocityProperty",&pragma::VelocityComponent::GetAngularVelocityProperty);
	entsMod[defVelocity];

	auto defGlobal = luabind::class_<pragma::GlobalNameComponent,pragma::BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName",&pragma::GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName",&pragma::GlobalNameComponent::SetGlobalName);
	entsMod[defGlobal];

	auto defComposite = luabind::class_<pragma::CompositeComponent,pragma::BaseEntityComponent>("CompositeComponent");
	defComposite.def("ClearEntities",&pragma::CompositeComponent::ClearEntities);
	defComposite.def("ClearEntities",&pragma::CompositeComponent::ClearEntities,luabind::default_parameter_policy<2,true>{});
	defComposite.def("ClearEntities",+[](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName) {
		auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities();
	});
	defComposite.def("ClearEntities",+[](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName,bool safely) {
		auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities(safely);
	});
	defComposite.def("GetEntities",+[](lua_State *l,pragma::CompositeComponent &hComponent) -> luabind::tableT<BaseEntity> {
		auto &ents = hComponent.GetRootCompositeGroup().GetEntities();
		auto tEnts = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
		}
		return tEnts;
	});
	defComposite.def("GetEntities",+[](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName) -> luabind::tableT<BaseEntity> {
		auto tEnts = luabind::newtable(l);
		auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		if(!group)
			return tEnts;
		auto &ents = group->GetEntities();
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
		}
		return tEnts;
	});
	defComposite.def("GetRootGroup",static_cast<pragma::CompositeGroup&(pragma::CompositeComponent::*)()>(&pragma::CompositeComponent::GetRootCompositeGroup));
	defComposite.def("AddEntity",+[](lua_State *l,pragma::CompositeComponent &hComponent,BaseEntity &ent) {
		hComponent.GetRootCompositeGroup().AddEntity(ent);
	});
	defComposite.def("AddEntity",+[](lua_State *l,pragma::CompositeComponent &hComponent,BaseEntity &ent,const std::string &groupName) {
		hComponent.GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(ent);
	});
	auto defCompositeGroup = luabind::class_<pragma::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity",&pragma::CompositeGroup::AddEntity);
	defCompositeGroup.def("AddEntity",+[](lua_State *l,pragma::CompositeGroup &hComponent,BaseEntity &ent,const std::string &groupName) {
		hComponent.AddChildGroup(groupName).AddEntity(ent);
	});
	defCompositeGroup.def("RemoveEntity",&pragma::CompositeGroup::RemoveEntity);
	defCompositeGroup.def("ClearEntities",&pragma::CompositeGroup::ClearEntities);
	defCompositeGroup.def("ClearEntities",&pragma::CompositeGroup::ClearEntities,luabind::default_parameter_policy<2,true>{});
	defCompositeGroup.def("ClearEntities",+[](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &groupName) {
		auto *group = hComponent.FindChildGroup(groupName);
		if(group)
			group->ClearEntities();
	});
	defCompositeGroup.def("ClearEntities",+[](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &groupName,bool safely) {
		auto *group = hComponent.FindChildGroup(groupName);
		if(group)
			group->ClearEntities(safely);
	});
	defCompositeGroup.def("GetEntities",+[](lua_State *l,pragma::CompositeGroup &hComponent) -> luabind::tableT<BaseEntity> {
		auto &ents = hComponent.GetEntities();
		auto tEnts = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
		}
		return tEnts;
	});
	defCompositeGroup.def("AddChildGroup",&pragma::CompositeGroup::AddChildGroup);
	defCompositeGroup.def("GetChildGroups",+[](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &name) -> luabind::tableT<pragma::CompositeGroup> {
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &childGroup : hComponent.GetChildGroups())
			t[idx++] = childGroup.get();
		return t;
	});
	defComposite.scope[defCompositeGroup];
	entsMod[defComposite];
	
	auto defAnimated2 = luabind::class_<pragma::Animated2Component,pragma::BaseEntityComponent>("Animated2Component");
	defAnimated2.scope[
		luabind::def("parse_component_channel_path",&pragma::Animated2Component::ParseComponentChannelPath)
	];
	defAnimated2.def("SetPlaybackRate",&pragma::Animated2Component::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate",&pragma::Animated2Component::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty",&pragma::Animated2Component::GetPlaybackRateProperty);
	defAnimated2.def("ClearAnimationManagers",&pragma::Animated2Component::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager",&pragma::Animated2Component::AddAnimationManager);
	defAnimated2.def("RemoveAnimationManager",&pragma::Animated2Component::RemoveAnimationManager);
	defAnimated2.def("GetAnimationManagers",+[](lua_State *l,pragma::Animated2Component &hComponent) -> luabind::tableT<panima::AnimationManager> {
		auto t = luabind::newtable(l);
		auto &animManagers = hComponent.GetAnimationManagers();
		for(auto i=decltype(animManagers.size()){0u};i<animManagers.size();++i)
			animManagers[i +1] = animManagers[i];
		return t;
	});
	defAnimated2.def("GetAnimationManager",+[](lua_State *l,pragma::Animated2Component &hComponent,uint32_t idx) -> luabind::tableT<panima::AnimationManager> {
		auto &animManagers = hComponent.GetAnimationManagers();
		if(idx >= animManagers.size())
			return luabind::object{};
		return luabind::object{l,animManagers[idx]};
	});
	defAnimated2.def("PlayAnimation",&pragma::Animated2Component::PlayAnimation);
	defAnimated2.def("ReloadAnimation",&pragma::Animated2Component::ReloadAnimation);
	defAnimated2.def("AdvanceAnimations",&pragma::Animated2Component::AdvanceAnimations);
	defAnimated2.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT",pragma::Animated2Component::EVENT_HANDLE_ANIMATION_EVENT);
	defAnimated2.add_static_constant("EVENT_ON_PLAY_ANIMATION",pragma::Animated2Component::EVENT_ON_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_COMPLETE",pragma::Animated2Component::EVENT_ON_ANIMATION_COMPLETE);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_START",pragma::Animated2Component::EVENT_ON_ANIMATION_START);
	defAnimated2.add_static_constant("EVENT_MAINTAIN_ANIMATIONS",pragma::Animated2Component::EVENT_MAINTAIN_ANIMATIONS);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED",pragma::Animated2Component::EVENT_ON_ANIMATIONS_UPDATED);
	defAnimated2.add_static_constant("EVENT_PLAY_ANIMATION",pragma::Animated2Component::EVENT_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_TRANSLATE_ANIMATION",pragma::Animated2Component::EVENT_TRANSLATE_ANIMATION);
	defAnimated2.add_static_constant("EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER",pragma::Animated2Component::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER);
	entsMod[defAnimated2];

	auto defDriverC = luabind::class_<pragma::AnimationDriverComponent,pragma::BaseEntityComponent>("AnimationDriverComponent");
	defDriverC.def("AddDriver",static_cast<
		void(*)(pragma::AnimationDriverComponent&,pragma::ComponentId,pragma::ComponentMemberIndex,pragma::ValueDriverDescriptor)
	>(&add_driver<pragma::ComponentMemberIndex>));
	defDriverC.def("AddDriver",static_cast<
		void(*)(pragma::AnimationDriverComponent&,pragma::ComponentId,const std::string&,pragma::ValueDriverDescriptor)
	>(&add_driver<const std::string&>));
	defDriverC.def("ClearDrivers",&pragma::AnimationDriverComponent::ClearDrivers);

	auto defDriver = luabind::class_<pragma::ValueDriver>("Driver");
	defDriver.def(luabind::constructor<pragma::ComponentId,pragma::ComponentMemberReference,pragma::ValueDriverDescriptor>(),ComponentMemberReferencePolicy<3>{});
	defDriver.def("GetMemberReference",&pragma::ValueDriver::GetMemberReference);
	defDriver.def("GetDescriptor",&pragma::ValueDriver::GetDescriptor);
	defDriverC.scope[defDriver];

	entsMod[defDriverC];

	auto defIK = luabind::class_<pragma::IKComponent,pragma::BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled",&pragma::IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled",&pragma::IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos",&pragma::IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos",&pragma::IKComponent::GetIKEffectorPos);
	entsMod[defIK];

	auto defLogic = luabind::class_<pragma::LogicComponent,pragma::BaseEntityComponent>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK",pragma::LogicComponent::EVENT_ON_TICK);
	entsMod[defLogic];

	auto defUsable = luabind::class_<pragma::UsableComponent,pragma::BaseEntityComponent>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE",pragma::UsableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE",pragma::UsableComponent::EVENT_CAN_USE);
	entsMod[defUsable];

	auto defMap = luabind::class_<pragma::MapComponent,pragma::BaseEntityComponent>("MapComponent");
	defMap.def("GetMapIndex",&pragma::MapComponent::GetMapIndex);
	entsMod[defMap];

	auto defSubmergible = luabind::class_<pragma::SubmergibleComponent,pragma::BaseEntityComponent>("SubmergibleComponent");
	defSubmergible.def("IsSubmerged",&pragma::SubmergibleComponent::IsSubmerged);
	defSubmergible.def("IsFullySubmerged",&pragma::SubmergibleComponent::IsFullySubmerged);
	defSubmergible.def("GetSubmergedFraction",&pragma::SubmergibleComponent::GetSubmergedFraction);
	defSubmergible.def("IsInWater",&pragma::SubmergibleComponent::IsInWater);
	defSubmergible.def("GetSubmergedFractionProperty",&pragma::SubmergibleComponent::GetSubmergedFractionProperty);
	defSubmergible.def("GetWaterEntity",static_cast<BaseEntity*(pragma::SubmergibleComponent::*)()>(&pragma::SubmergibleComponent::GetWaterEntity));
	defSubmergible.add_static_constant("EVENT_ON_WATER_SUBMERGED",pragma::SubmergibleComponent::EVENT_ON_WATER_SUBMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EMERGED",pragma::SubmergibleComponent::EVENT_ON_WATER_EMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_ENTERED",pragma::SubmergibleComponent::EVENT_ON_WATER_ENTERED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EXITED",pragma::SubmergibleComponent::EVENT_ON_WATER_EXITED);
	entsMod[defSubmergible];

	auto defDamageable = luabind::class_<pragma::DamageableComponent,pragma::BaseEntityComponent>("DamageableComponent");
	defDamageable.def("TakeDamage",&pragma::DamageableComponent::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE",pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE);
	entsMod[defDamageable];
}
#pragma optimize("",on)
