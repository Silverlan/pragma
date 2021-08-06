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
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/model/animation/animation_manager.hpp"
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
#include "pragma/lua/classes/entity_components.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/policies/optional_policy.hpp"
#include <pragma/physics/movetypes.h>
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

namespace util
{
	template<typename T>
		class TWeakSharedHandle;
	template<typename T>
		class TSharedHandle;
	template<typename T>
		class WeakHandle;
};
namespace luabind
{
	namespace detail
	{
		template<typename T>
		struct pointer_traits<util::WeakHandle<T>>
		{
			enum { is_pointer = true };
			using value_type = T;
		};
	};
};

namespace luabind {

	namespace detail
	{

		struct weak_handle_deleter
		{
			weak_handle_deleter(lua_State* L, int index)
				: life_support(get_main_thread(L), L, index)
			{}

			void operator()(void const*)
			{
				handle().swap(life_support);
			}

			handle life_support;
		};

	} // namespace detail

	template <class T>
	struct default_converter<util::WeakHandle<T> >
		: default_converter<T*>
	{
		using is_native = std::false_type;

		template <class U>
		int match(lua_State* L, U, int index)
		{
			return default_converter<T*>::match(L, decorate_type_t<T*>(), index);
		}

		template <class U>
		util::WeakHandle<T> to_cpp(lua_State* L, U, int index)
		{
			T* raw_ptr = default_converter<T*>::to_cpp(L, decorate_type_t<T*>(), index);

			if(!raw_ptr) {
				return util::WeakHandle<T>();
			} else {
				return util::WeakHandle<T>(raw_ptr, detail::weak_handle_deleter(L, index));
			}
		}

		void to_lua(lua_State* L, util::WeakHandle<T> const& p)
		{
			if(detail::weak_handle_deleter* d = nullptr)
			{
				d->life_support.push(L);
			} else {
				detail::value_converter().to_lua(L, p);
			}
		}

		template <class U>
		void converter_postcall(lua_State*, U const&, int)
		{}
	};

	template <class T>
	struct default_converter<util::WeakHandle<T> const&>
		: default_converter<util::WeakHandle<T> >
	{};

	template <typename T>
	struct lua_proxy_traits<util::WeakHandle<T> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx) && lua_istable(L, idx) && lua_isboolean(L,idx);
		}
	};

	//////

	namespace adl {
		template <typename T>
		struct TestWrapper : object
		{
			TestWrapper(from_stack const& stack_reference)
				: object(stack_reference)
			{}
			TestWrapper(const object &o)
				: object(o)
			{}
			TestWrapper(lua_State *l,const T &t)
				: object(l,t)
			{}
			using value_type = T;
		};
	};

	namespace detail
	{
		template<typename T>
		struct pseudo_traits<adl::TestWrapper<T>>
		{
			enum { is_pseudo_type = true };
			enum { is_variadic = false };
			using value_type = T;
		};

	};
	using adl::TestWrapper;

	template <typename T>
	struct lua_proxy_traits<adl::TestWrapper<T> >
		: lua_proxy_traits<object>
	{
		static bool check(lua_State* L, int idx)
		{
			return lua_proxy_traits<object>::check(L, idx) && lua_istable(L,idx);//(lua_isnoneornil(L, idx) || lua_isnumber(L, idx));
		}
	};

} // namespace luabind

void Game::RegisterLuaEntityComponents(luabind::module_ &gameMod)
{
	pragma::lua::register_entity_component_classes(gameMod);
	Lua::register_gravity_component(gameMod);

	auto defVelocity = luabind::class_<pragma::VelocityComponent,pragma::BaseEntityComponent>("VelocityComponent");
	defVelocity.def("GetVelocity",&pragma::VelocityComponent::GetVelocity);
	defVelocity.def("SetVelocity",&pragma::VelocityComponent::SetVelocity);
	defVelocity.def("AddVelocity",&pragma::VelocityComponent::AddVelocity);
	defVelocity.def("GetAngularVelocity",&pragma::VelocityComponent::GetAngularVelocity);
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
	gameMod[defVelocity];

	auto defGlobal = luabind::class_<pragma::GlobalNameComponent,pragma::BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName",&pragma::GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName",&pragma::GlobalNameComponent::SetGlobalName);
	gameMod[defGlobal];

	auto defComposite = luabind::class_<pragma::CompositeComponent,pragma::BaseEntityComponent>("CompositeComponent");
	defComposite.def("ClearEntities",&pragma::CompositeComponent::ClearEntities);
	defComposite.def("ClearEntities",&pragma::CompositeComponent::ClearEntities,luabind::default_parameter_policy<2,true>{});
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeComponent&,const std::string&)>([](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName) {
		auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities();
	}));
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeComponent&,const std::string&,bool)>([](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName,bool safely) {
		auto *group = hComponent.GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities(safely);
	}));
	defComposite.def("GetEntities",static_cast<luabind::tableT<BaseEntity>(*)(lua_State*,pragma::CompositeComponent&)>([](lua_State *l,pragma::CompositeComponent &hComponent) -> luabind::tableT<BaseEntity> {
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
	}));
	defComposite.def("GetEntities",static_cast<luabind::tableT<BaseEntity>(*)(lua_State*,pragma::CompositeComponent&,const std::string&)>([](lua_State *l,pragma::CompositeComponent &hComponent,const std::string &groupName) -> luabind::tableT<BaseEntity> {
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
	}));
	defComposite.def("GetRootGroup",static_cast<pragma::CompositeGroup&(pragma::CompositeComponent::*)()>(&pragma::CompositeComponent::GetRootCompositeGroup));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeComponent&,BaseEntity&)>([](lua_State *l,pragma::CompositeComponent &hComponent,BaseEntity &ent) {
		hComponent.GetRootCompositeGroup().AddEntity(ent);
	}));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeComponent&,BaseEntity&,const std::string&)>([](lua_State *l,pragma::CompositeComponent &hComponent,BaseEntity &ent,const std::string &groupName) {
		hComponent.GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(ent);
	}));
	auto defCompositeGroup = luabind::class_<pragma::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity",&pragma::CompositeGroup::AddEntity);
	defCompositeGroup.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,BaseEntity&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,BaseEntity &ent,const std::string &groupName) {
		hComponent.AddChildGroup(groupName).AddEntity(ent);
	}));
	defCompositeGroup.def("RemoveEntity",&pragma::CompositeGroup::RemoveEntity);
	defCompositeGroup.def("ClearEntities",&pragma::CompositeGroup::ClearEntities);
	defCompositeGroup.def("ClearEntities",&pragma::CompositeGroup::ClearEntities,luabind::default_parameter_policy<2,true>{});
	defCompositeGroup.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &groupName) {
		auto *group = hComponent.FindChildGroup(groupName);
		if(group)
			group->ClearEntities();
	}));
	defCompositeGroup.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,const std::string&,bool)>([](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &groupName,bool safely) {
		auto *group = hComponent.FindChildGroup(groupName);
		if(group)
			group->ClearEntities(safely);
	}));
	defCompositeGroup.def("GetEntities",static_cast<luabind::tableT<BaseEntity>(*)(lua_State*,pragma::CompositeGroup&)>([](lua_State *l,pragma::CompositeGroup &hComponent) -> luabind::tableT<BaseEntity> {
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
	}));
	defCompositeGroup.def("AddChildGroup",&pragma::CompositeGroup::AddChildGroup);
	defCompositeGroup.def("GetChildGroups",static_cast<luabind::tableT<pragma::CompositeGroup>(*)(lua_State*,pragma::CompositeGroup&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &name) -> luabind::tableT<pragma::CompositeGroup> {
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &childGroup : hComponent.GetChildGroups())
			t[idx++] = childGroup.get();
		return t;
	}));
	defComposite.scope[defCompositeGroup];
	gameMod[defComposite];
	
	auto defAnimated2 = luabind::class_<pragma::Animated2Component,pragma::BaseEntityComponent>("Animated2Component");
	defAnimated2.def("SetPlaybackRate",&pragma::Animated2Component::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate",&pragma::Animated2Component::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty",&pragma::Animated2Component::GetPlaybackRateProperty);
	defAnimated2.def("ClearAnimationManagers",&pragma::Animated2Component::ClearAnimationManagers);
	defAnimated2.def("AddAnimationManager",&pragma::Animated2Component::AddAnimationManager);
	defAnimated2.def("RemoveAnimationManager",&pragma::Animated2Component::RemoveAnimationManager);
	defAnimated2.def("GetAnimationManagers",static_cast<luabind::tableT<pragma::animation::AnimationManager>(*)(lua_State*,pragma::Animated2Component&)>([](lua_State *l,pragma::Animated2Component &hComponent) -> luabind::tableT<pragma::animation::AnimationManager> {
		auto t = luabind::newtable(l);
		auto &animManagers = hComponent.GetAnimationManagers();
		for(auto i=decltype(animManagers.size()){0u};i<animManagers.size();++i)
			animManagers[i +1] = animManagers[i];
		return t;
	}));
	defAnimated2.def("GetAnimationManager",static_cast<luabind::tableT<pragma::animation::AnimationManager>(*)(lua_State*,pragma::Animated2Component&,uint32_t)>([](lua_State *l,pragma::Animated2Component &hComponent,uint32_t idx) -> luabind::tableT<pragma::animation::AnimationManager> {
		auto &animManagers = hComponent.GetAnimationManagers();
		if(idx >= animManagers.size())
			return luabind::object{};
		return luabind::object{l,animManagers[idx]};
	}));
	gameMod[defAnimated2];

	auto defIK = luabind::class_<pragma::IKComponent,pragma::BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled",&pragma::IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled",&pragma::IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos",&pragma::IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos",&pragma::IKComponent::GetIKEffectorPos);
	gameMod[defIK];

	auto defLogic = luabind::class_<pragma::LogicComponent,pragma::BaseEntityComponent>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK",pragma::LogicComponent::EVENT_ON_TICK);
	gameMod[defLogic];

	auto defUsable = luabind::class_<pragma::UsableComponent,pragma::BaseEntityComponent>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE",pragma::UsableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE",pragma::UsableComponent::EVENT_CAN_USE);
	gameMod[defUsable];

	auto defMap = luabind::class_<pragma::MapComponent,pragma::BaseEntityComponent>("MapComponent");
	defMap.def("GetMapIndex",&pragma::MapComponent::GetMapIndex);
	gameMod[defMap];

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
	gameMod[defSubmergible];

	auto defDamageable = luabind::class_<pragma::DamageableComponent,pragma::BaseEntityComponent>("DamageableComponent");
	defDamageable.def("TakeDamage",&pragma::DamageableComponent::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE",pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE);
	gameMod[defDamageable];
}
#pragma optimize("",on)
