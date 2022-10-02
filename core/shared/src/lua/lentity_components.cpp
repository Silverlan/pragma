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
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/physics/raytraces.h"
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

static int lua_match_component_member_reference(lua_State *l,int index) {return lua_isstring(l,index) ? 0 : luabind::no_match;}
static pragma::ComponentMemberReference lua_to_component_member_reference(lua_State *l,int index) {return pragma::ComponentMemberReference{Lua::CheckString(l,index)};}

template<uint32_t N>
	using ComponentMemberReferencePolicy = luabind::generic_policy<
		N,pragma::ComponentMemberReference,
		&lua_match_component_member_reference,
		&lua_to_component_member_reference
	>;

/*template<uint32_t N>
	using UniversalReferencePolicy = luabind::generic_policy<N,util::Uuid,[](lua_State *l,int index) -> int {
		return lua_isstring(l,index) ? 0 : luabind::no_match;
	},[](lua_State *l,int index) -> util::Uuid {
		return util::uuid_string_to_bytes(Lua::CheckString(l,index));
	}>;
*/

static std::string to_string(Game &game,const pragma::EntityURef &ref)
{
	std::stringstream ss;
	ss<<"[";
	auto *ent = ref.GetEntity(game);
	if(!ent)
		ss<<"NULL";
	else
		const_cast<BaseEntity*>(ent)->print(ss);
	ss<<"]";

	auto *identifier = ref.GetIdentifier();
	ss<<"[";
	if(!identifier)
		ss<<"NULL";
	else
	{
		std::visit([&ss](auto &val) {
			using T = util::base_type<decltype(val)>;
			if constexpr(std::is_same_v<T,util::Uuid>)
				ss<<"Uuid:"<<util::uuid_to_string(val);
			else
				ss<<"Name:"<<val;
		},*identifier);
	}
	ss<<"]";
	return ss.str();
}

static std::string to_string(Game &game,const pragma::EntityUComponentRef &ref)
{
	std::stringstream ss;
	ss<<to_string(game,static_cast<const pragma::EntityURef&>(ref));
	auto componentId = ref.GetComponentId();
	if(componentId != pragma::INVALID_COMPONENT_ID)
		ss<<"[ComponentId:"<<componentId<<"]";
	else
	{
		auto *name = ref.GetComponentName();
		if(name)
			ss<<"[ComponentName:"<<*name<<"]";
		else
			ss<<"[Component:NULL]";
	}
	return ss.str();
}

static std::string to_string(Game &game,const pragma::EntityUComponentMemberRef &ref)
{
	std::stringstream ss;
	ss<<to_string(game,static_cast<const pragma::EntityURef&>(ref));
	ss<<"[";
	auto *memberInfo = ref.GetMemberInfo(game);
	if(memberInfo)
		ss<<"Member:"<<memberInfo->GetName();
	else
		ss<<"Member:NULL";
	ss<<"]";
	return ss.str();
}

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma,BaseEntityComponent);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma,ValueDriver);

void Game::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	pragma::lua::register_entity_component_classes(entsMod);
	Lua::register_gravity_component(entsMod);

	auto classDefEntRef = luabind::class_<pragma::EntityURef>("UniversalEntityReference");
	classDefEntRef.def(luabind::constructor<const BaseEntity&>());
	classDefEntRef.def(luabind::constructor<util::Uuid>());
	classDefEntRef.def(luabind::constructor<const std::string&>());
	classDefEntRef.def("GetEntity",static_cast<BaseEntity*(pragma::EntityURef::*)(Game&)>(&pragma::EntityURef::GetEntity));
	classDefEntRef.def("__tostring",+[](Game &game,const pragma::EntityURef &ref) -> std::string {
		std::stringstream ss;
		ss<<"UniversalEntityReference"<<to_string(game,ref);
		return ss.str();
	});
	entsMod[classDefEntRef];

	auto classDefCompRef = luabind::class_<pragma::EntityUComponentRef,pragma::EntityURef>("UniversalComponentReference");
	classDefCompRef.def(luabind::constructor<util::Uuid,pragma::ComponentId>());
	classDefCompRef.def(luabind::constructor<const std::string&,pragma::ComponentId>());
	classDefCompRef.def(luabind::constructor<util::Uuid,const std::string&>());
	classDefCompRef.def(luabind::constructor<const std::string&,const std::string&>());
	classDefCompRef.def(luabind::constructor<const BaseEntity&,pragma::ComponentId>());
	classDefCompRef.def("GetComponent",static_cast<pragma::BaseEntityComponent*(pragma::EntityUComponentRef::*)(Game&)>(&pragma::EntityUComponentRef::GetComponent));
	classDefCompRef.def("__tostring",+[](Game &game,const pragma::EntityUComponentRef &ref) -> std::string {
		std::stringstream ss;
		ss<<"UniversalComponentReference"<<to_string(game,ref);
		return ss.str();
	});
	entsMod[classDefCompRef];

	auto classDefMemRef = luabind::class_<pragma::EntityUComponentMemberRef,luabind::bases<pragma::EntityUComponentRef,pragma::EntityURef>>("UniversalMemberReference");
	classDefMemRef.def(luabind::constructor<util::Uuid,pragma::ComponentId,const std::string&>());
	classDefMemRef.def(luabind::constructor<const std::string&,pragma::ComponentId,const std::string&>());
	classDefMemRef.def(luabind::constructor<util::Uuid,const std::string&,const std::string&>());
	classDefMemRef.def(luabind::constructor<const std::string&,const std::string&,const std::string&>());
	classDefMemRef.def(luabind::constructor<const BaseEntity&,pragma::ComponentId,const std::string&>());
	classDefMemRef.def(luabind::constructor<const BaseEntity&,const std::string&,const std::string&>());
	classDefMemRef.def("GetMemberInfo",&pragma::EntityUComponentMemberRef::GetMemberInfo);
	classDefMemRef.def("__tostring",+[](Game &game,const pragma::EntityUComponentMemberRef &ref) -> std::string {
		std::stringstream ss;
		ss<<"UniversalMemberReference"<<to_string(game,ref);
		return ss.str();
	});
	entsMod[classDefMemRef];

	auto classDefMultiEntRef = luabind::class_<pragma::MultiEntityURef>("MultiUniversalEntityReference");
	classDefMultiEntRef.def(luabind::constructor<const BaseEntity&>());
	classDefMultiEntRef.def(luabind::constructor<util::Uuid>());
	classDefMultiEntRef.def(luabind::constructor<const std::string&>());
	classDefMultiEntRef.def("FindEntities",&pragma::MultiEntityURef::FindEntities);
	entsMod[classDefMultiEntRef];

	auto defVelocity = pragma::lua::create_entity_component_class<pragma::VelocityComponent,pragma::BaseEntityComponent>("VelocityComponent");
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

	auto defGlobal = pragma::lua::create_entity_component_class<pragma::GlobalNameComponent,pragma::BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName",&pragma::GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName",&pragma::GlobalNameComponent::SetGlobalName);
	entsMod[defGlobal];

	auto defComposite = pragma::lua::create_entity_component_class<pragma::CompositeComponent,pragma::BaseEntityComponent>("CompositeComponent");
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
	
	auto defAnimated2 = pragma::lua::create_entity_component_class<pragma::PanimaComponent,pragma::BaseEntityComponent>("PanimaComponent");
	defAnimated2.scope[
		luabind::def("parse_component_channel_path",&pragma::PanimaComponent::ParseComponentChannelPath)
	];
	defAnimated2.def("ReloadAnimation",static_cast<void(pragma::PanimaComponent::*)()>(&pragma::PanimaComponent::ReloadAnimation));
	defAnimated2.def("SetPlaybackRate",&pragma::PanimaComponent::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate",&pragma::PanimaComponent::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty",&pragma::PanimaComponent::GetPlaybackRateProperty);
	defAnimated2.def("GetCurrentTime",&pragma::PanimaComponent::GetCurrentTime);
	defAnimated2.def("SetCurrentTime",&pragma::PanimaComponent::SetCurrentTime);
	defAnimated2.def("GetCurrentTimeFraction",&pragma::PanimaComponent::GetCurrentTimeFraction);
	defAnimated2.def("SetCurrentTimeFraction",&pragma::PanimaComponent::SetCurrentTimeFraction);
	defAnimated2.def("ClearAnimationManagers",&pragma::PanimaComponent::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager",&pragma::PanimaComponent::AddAnimationManager);
	defAnimated2.def("RemoveAnimationManager",static_cast<void(pragma::PanimaComponent::*)(const panima::AnimationManager&)>(&pragma::PanimaComponent::RemoveAnimationManager));
	defAnimated2.def("RemoveAnimationManager",static_cast<void(pragma::PanimaComponent::*)(const std::string_view&)>(&pragma::PanimaComponent::RemoveAnimationManager));
	defAnimated2.def("GetAnimationManagers",+[](lua_State *l,pragma::PanimaComponent &hComponent) -> luabind::tableT<panima::AnimationManager> {
		auto t = luabind::newtable(l);
		auto &animManagers = hComponent.GetAnimationManagers();
		for(auto &pair : animManagers)
			t[pair.first] = pair.second;
		return t;
	});
	defAnimated2.def("GetAnimationManager",+[](lua_State *l,pragma::PanimaComponent &hComponent,const std::string &name) {
		return hComponent.GetAnimationManager(name);
	});
	defAnimated2.def("PlayAnimation",&pragma::PanimaComponent::PlayAnimation);
	defAnimated2.def("ReloadAnimation",static_cast<void(pragma::PanimaComponent::*)(panima::AnimationManager&)>(&pragma::PanimaComponent::ReloadAnimation));
	defAnimated2.def("AdvanceAnimations",&pragma::PanimaComponent::AdvanceAnimations);
	defAnimated2.def("DebugPrint",static_cast<void(pragma::PanimaComponent::*)()>(&pragma::PanimaComponent::DebugPrint));
	defAnimated2.add_static_constant("EVENT_HANDLE_ANIMATION_EVENT",pragma::PanimaComponent::EVENT_HANDLE_ANIMATION_EVENT);
	defAnimated2.add_static_constant("EVENT_ON_PLAY_ANIMATION",pragma::PanimaComponent::EVENT_ON_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_COMPLETE",pragma::PanimaComponent::EVENT_ON_ANIMATION_COMPLETE);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATION_START",pragma::PanimaComponent::EVENT_ON_ANIMATION_START);
	defAnimated2.add_static_constant("EVENT_MAINTAIN_ANIMATIONS",pragma::PanimaComponent::EVENT_MAINTAIN_ANIMATIONS);
	defAnimated2.add_static_constant("EVENT_ON_ANIMATIONS_UPDATED",pragma::PanimaComponent::EVENT_ON_ANIMATIONS_UPDATED);
	defAnimated2.add_static_constant("EVENT_PLAY_ANIMATION",pragma::PanimaComponent::EVENT_PLAY_ANIMATION);
	defAnimated2.add_static_constant("EVENT_TRANSLATE_ANIMATION",pragma::PanimaComponent::EVENT_TRANSLATE_ANIMATION);
	defAnimated2.add_static_constant("EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER",pragma::PanimaComponent::EVENT_INITIALIZE_CHANNEL_VALUE_SUBMITTER);
	entsMod[defAnimated2];

	auto defDriverC = pragma::lua::create_entity_component_class<pragma::AnimationDriverComponent,pragma::BaseEntityComponent>("AnimationDriverComponent");
	defDriverC.def("AddDriver",static_cast<
		void(*)(pragma::AnimationDriverComponent&,pragma::ComponentId,pragma::ComponentMemberIndex,pragma::ValueDriverDescriptor)
	>(&add_driver<pragma::ComponentMemberIndex>));
	defDriverC.def("AddDriver",static_cast<
		void(*)(pragma::AnimationDriverComponent&,pragma::ComponentId,const std::string&,pragma::ValueDriverDescriptor)
	>(&add_driver<const std::string&>));
	defDriverC.def("ClearDrivers",&pragma::AnimationDriverComponent::ClearDrivers);

	auto defDriver = luabind::class_<pragma::ValueDriver>("Driver");
	defDriver.def(luabind::tostring(luabind::self));
	defDriver.def("GetMemberReference",&pragma::ValueDriver::GetMemberReference);
	defDriver.def("GetDescriptor",&pragma::ValueDriver::GetDescriptor);
	defDriverC.scope[defDriver];
	entsMod[defDriverC];
	pragma::lua::define_custom_constructor<pragma::ValueDriver,[](pragma::ComponentId componentId,const std::string &memberRef,pragma::ValueDriverDescriptor descriptor,const std::string &self) -> pragma::ValueDriver {
		return pragma::ValueDriver{componentId,memberRef,descriptor,util::uuid_string_to_bytes(self)};
	},pragma::ComponentId,const std::string&,pragma::ValueDriverDescriptor,const std::string&>(GetLuaState());

	auto defIK = pragma::lua::create_entity_component_class<pragma::IKComponent,pragma::BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled",&pragma::IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled",&pragma::IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos",&pragma::IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos",&pragma::IKComponent::GetIKEffectorPos);
	entsMod[defIK];

	auto defLogic = pragma::lua::create_entity_component_class<pragma::LogicComponent,pragma::BaseEntityComponent>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK",pragma::LogicComponent::EVENT_ON_TICK);
	entsMod[defLogic];

	auto defUsable = pragma::lua::create_entity_component_class<pragma::UsableComponent,pragma::BaseEntityComponent>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE",pragma::UsableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE",pragma::UsableComponent::EVENT_CAN_USE);
	entsMod[defUsable];

	auto defMap = pragma::lua::create_entity_component_class<pragma::MapComponent,pragma::BaseEntityComponent>("MapComponent");
	defMap.def("GetMapIndex",&pragma::MapComponent::GetMapIndex);
	entsMod[defMap];

	auto defSubmergible = pragma::lua::create_entity_component_class<pragma::SubmergibleComponent,pragma::BaseEntityComponent>("SubmergibleComponent");
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

	auto defDamageable = pragma::lua::create_entity_component_class<pragma::DamageableComponent,pragma::BaseEntityComponent>("DamageableComponent");
	defDamageable.def("TakeDamage",&pragma::DamageableComponent::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE",pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE);
	entsMod[defDamageable];
}
