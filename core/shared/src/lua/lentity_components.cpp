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
#include "pragma/util/bulletinfo.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/model/animation/animation_manager.hpp"
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/lua/lentity_components_base_types.hpp"
#include "pragma/entities/components/animated_2_component.hpp"
#include <pragma/physics/movetypes.h>
#pragma optimize("",off)
namespace Lua
{
	namespace Velocity
	{
		static void GetVelocity(lua_State *l,VelocityHandle &hEnt);
		static void SetVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel);
		static void AddVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel);
		static void GetAngularVelocity(lua_State *l,VelocityHandle &hEnt);
		static void SetAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel);
		static void AddAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel);
		static void GetLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt);
		static void SetLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel);
		static void AddLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel);
		static void GetLocalVelocity(lua_State *l,VelocityHandle &hEnt);
		static void SetLocalVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel);
		static void AddLocalVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel);
	};
	namespace Damageable
	{
		static void TakeDamage(lua_State *l,DamageableHandle &hEnt,DamageInfo &info);
	};
	namespace Submergible
	{
		// Water
		static void IsSubmerged(lua_State *l,SubmergibleHandle &hEnt);
		static void IsFullySubmerged(lua_State *l,SubmergibleHandle &hEnt);
		static void GetSubmergedFraction(lua_State *l,SubmergibleHandle &hEnt);
		static void IsInWater(lua_State *l,SubmergibleHandle &hEnt);
	};
	namespace IK
	{
		static void SetIKControllerEnabled(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,bool b);
		static void IsIKControllerEnabled(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId);
		static void SetIKEffectorPos(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,uint32_t effectorIdx,const Vector3 &pos);
		static void GetIKEffectorPos(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,uint32_t effectorIdx);
	};
};

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
	auto def = luabind::class_<BaseEntityComponentHandle>("EntityComponent");
	RegisterLuaEntityComponent(def);
	gameMod[def];

	Lua::register_gravity_component(gameMod);

	//pragma::VelocityComponent *x;
	//x->GetHandle<pragma::VelocityComponent>();
	//util::WeakHandle<pragma::VelocityComponent>();

	auto defVelocity = luabind::class_<VelocityHandle,BaseEntityComponentHandle>("VelocityComponent");
	defVelocity.def("GetVelocity",&Lua::Velocity::GetVelocity);
	defVelocity.def("SetVelocity",&Lua::Velocity::SetVelocity);
	defVelocity.def("AddVelocity",&Lua::Velocity::AddVelocity);
	defVelocity.def("GetAngularVelocity",&Lua::Velocity::GetAngularVelocity);
	defVelocity.def("SetAngularVelocity",&Lua::Velocity::SetAngularVelocity);
	defVelocity.def("AddAngularVelocity",&Lua::Velocity::AddAngularVelocity);
	defVelocity.def("GetLocalAngularVelocity",&Lua::Velocity::GetLocalAngularVelocity);
	defVelocity.def("SetLocalAngularVelocity",&Lua::Velocity::SetLocalAngularVelocity);
	defVelocity.def("AddLocalAngularVelocity",&Lua::Velocity::AddLocalAngularVelocity);
	defVelocity.def("GetLocalVelocity",&Lua::Velocity::GetLocalVelocity);
	defVelocity.def("SetLocalVelocity",&Lua::Velocity::SetLocalVelocity);
	defVelocity.def("AddLocalVelocity",&Lua::Velocity::AddLocalVelocity);
	defVelocity.def("GetVelocityProperty",static_cast<void(*)(lua_State*,VelocityHandle&)>([](lua_State *l,VelocityHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetVelocityProperty());
	}));
	defVelocity.def("GetAngularVelocityProperty",static_cast<void(*)(lua_State*,VelocityHandle&)>([](lua_State *l,VelocityHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetAngularVelocityProperty());
	}));
	gameMod[defVelocity];

	auto defGlobal = luabind::class_<GlobalNameHandle,BaseEntityComponentHandle>("GlobalComponent");
	defGlobal.def("GetGlobalName",static_cast<void(*)(lua_State*,GlobalNameHandle&)>([](lua_State *l,GlobalNameHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushString(l,hComponent->GetGlobalName());
	}));
	defGlobal.def("SetGlobalName",static_cast<void(*)(lua_State*,GlobalNameHandle&,const std::string&)>([](lua_State *l,GlobalNameHandle &hComponent,const std::string &globalName) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetGlobalName(globalName);
	}));
	gameMod[defGlobal];

	auto defComposite = luabind::class_<CompositeHandle,BaseEntityComponentHandle>("CompositeComponent");
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,CompositeHandle&)>([](lua_State *l,CompositeHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearEntities();
	}));
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,CompositeHandle&,bool)>([](lua_State *l,CompositeHandle &hComponent,bool safely) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearEntities(safely);
	}));
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,CompositeHandle&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,const std::string &groupName) {
		pragma::Lua::check_component(l,hComponent);
		auto *group = hComponent->GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities();
	}));
	defComposite.def("ClearEntities",static_cast<void(*)(lua_State*,CompositeHandle&,const std::string&,bool)>([](lua_State *l,CompositeHandle &hComponent,const std::string &groupName,bool safely) {
		pragma::Lua::check_component(l,hComponent);
		auto *group = hComponent->GetRootCompositeGroup().FindChildGroup(groupName);
		if(group)
			group->ClearEntities(safely);
	}));
	defComposite.def("GetEntities",static_cast<luabind::object(*)(lua_State*,CompositeHandle&)>([](lua_State *l,CompositeHandle &hComponent) -> luabind::object {
		pragma::Lua::check_component(l,hComponent);
		auto &ents = hComponent->GetRootCompositeGroup().GetEntities();
		auto tEnts = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.IsValid())
				continue;
			tEnts[idx++] = *hEnt.get()->GetLuaObject();
		}
		return tEnts;
	}));
	defComposite.def("GetEntities",static_cast<luabind::object(*)(lua_State*,CompositeHandle&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,const std::string &groupName) -> luabind::object {
		pragma::Lua::check_component(l,hComponent);
		auto tEnts = luabind::newtable(l);
		auto *group = hComponent->GetRootCompositeGroup().FindChildGroup(groupName);
		if(!group)
			return tEnts;
		auto &ents = group->GetEntities();
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.IsValid())
				continue;
			tEnts[idx++] = *hEnt.get()->GetLuaObject();
		}
		return tEnts;
	}));
	defComposite.def("GetRootGroup",static_cast<pragma::CompositeGroup&(*)(lua_State*,CompositeHandle&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,const std::string &name) -> pragma::CompositeGroup& {
		pragma::Lua::check_component(l,hComponent);
		return hComponent->GetRootCompositeGroup();
	}));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,CompositeHandle&,EntityHandle&)>([](lua_State *l,CompositeHandle &hComponent,EntityHandle &hEnt) {
		pragma::Lua::check_component(l,hComponent);
		LUA_CHECK_ENTITY(l,hEnt);
		hComponent->GetRootCompositeGroup().AddEntity(*hEnt.get());
	}));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,CompositeHandle&,EntityHandle&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,EntityHandle &hEnt,const std::string &groupName) {
		pragma::Lua::check_component(l,hComponent);
		LUA_CHECK_ENTITY(l,hEnt);
		hComponent->GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(*hEnt.get());
	}));
	auto defCompositeGroup = luabind::class_<pragma::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,EntityHandle&)>([](lua_State *l,pragma::CompositeGroup &hComponent,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hComponent.AddEntity(*hEnt.get());
	}));
	defCompositeGroup.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,EntityHandle&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,EntityHandle &hEnt,const std::string &groupName) {
		LUA_CHECK_ENTITY(l,hEnt);
		hComponent.AddChildGroup(groupName).AddEntity(*hEnt.get());
	}));
	defCompositeGroup.def("RemoveEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,EntityHandle&)>([](lua_State *l,pragma::CompositeGroup &hComponent,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hComponent.RemoveEntity(*hEnt.get());
	}));
	defCompositeGroup.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeGroup&)>([](lua_State *l,pragma::CompositeGroup &hComponent) {
		hComponent.ClearEntities();
	}));
	defCompositeGroup.def("ClearEntities",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,bool)>([](lua_State *l,pragma::CompositeGroup &hComponent,bool safely) {
		hComponent.ClearEntities(safely);
	}));
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
	defCompositeGroup.def("GetEntities",static_cast<luabind::object(*)(lua_State*,pragma::CompositeGroup&)>([](lua_State *l,pragma::CompositeGroup &hComponent) -> luabind::object {
		auto &ents = hComponent.GetEntities();
		auto tEnts = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &hEnt : ents)
		{
			if(!hEnt.IsValid())
				continue;
			tEnts[idx++] = *hEnt.get()->GetLuaObject();
		}
		return tEnts;
	}));
	defCompositeGroup.def("AddChildGroup",static_cast<pragma::CompositeGroup&(*)(lua_State*,pragma::CompositeGroup&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &name) -> pragma::CompositeGroup& {
		return hComponent.AddChildGroup(name);
	}));
	defCompositeGroup.def("GetChildGroups",static_cast<luabind::object(*)(lua_State*,pragma::CompositeGroup&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,const std::string &name) -> luabind::object {
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &childGroup : hComponent.GetChildGroups())
			t[idx++] = childGroup.get();
		return t;
	}));
	defComposite.scope[defCompositeGroup];
	gameMod[defComposite];
	
	auto defAnimated2 = luabind::class_<Animated2Handle,BaseEntityComponentHandle>("Animated2Component");
	defAnimated2.def("SetPlaybackRate",static_cast<void(*)(lua_State*,Animated2Handle&,float)>([](lua_State *l,Animated2Handle &hAnim,float playbackRate) {
		pragma::Lua::check_component(l,hAnim);
		hAnim->SetPlaybackRate(playbackRate);
	}));
	defAnimated2.def("GetPlaybackRate",static_cast<float(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hAnim) -> float {
		pragma::Lua::check_component(l,hAnim);
		return hAnim->GetPlaybackRate();
	}));
	defAnimated2.def("GetPlaybackRateProperty",static_cast<void(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetPlaybackRateProperty());
	}));
	defAnimated2.def("ClearAnimationManagers",static_cast<void(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearAnimationManagers();
	}));
	defAnimated2.def("AddAnimationManager",static_cast<pragma::animation::PAnimationManager(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hComponent) -> pragma::animation::PAnimationManager {
		pragma::Lua::check_component(l,hComponent);
		return hComponent->AddAnimationManager();
	}));
	defAnimated2.def("RemoveAnimationManager",static_cast<void(*)(lua_State*,Animated2Handle&,const pragma::animation::AnimationManager&)>([](lua_State *l,Animated2Handle &hComponent,const pragma::animation::AnimationManager &manager) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->RemoveAnimationManager(manager);
	}));
	defAnimated2.def("GetAnimationManagers",static_cast<luabind::object(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto t = luabind::newtable(l);
		auto &animManagers = hComponent->GetAnimationManagers();
		for(auto i=decltype(animManagers.size()){0u};i<animManagers.size();++i)
			animManagers[i +1] = animManagers[i];
		return t;
	}));
	defAnimated2.def("GetAnimationManager",static_cast<luabind::object(*)(lua_State*,Animated2Handle&,uint32_t)>([](lua_State *l,Animated2Handle &hComponent,uint32_t idx) {
		pragma::Lua::check_component(l,hComponent);
		auto &animManagers = hComponent->GetAnimationManagers();
		if(idx >= animManagers.size())
			return luabind::object{};
		return luabind::object{l,animManagers[idx]};
	}));
	gameMod[defAnimated2];

	auto defIK = luabind::class_<IKHandle,BaseEntityComponentHandle>("IKComponent");
	defIK.def("SetIKControllerEnabled",&Lua::IK::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled",&Lua::IK::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos",&Lua::IK::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos",&Lua::IK::GetIKEffectorPos);
	gameMod[defIK];

	auto defLogic = luabind::class_<LogicHandle,BaseEntityComponentHandle>("LogicComponent");
	defLogic.add_static_constant("EVENT_ON_TICK",pragma::LogicComponent::EVENT_ON_TICK);
	gameMod[defLogic];

	auto defUsable = luabind::class_<UsableHandle,BaseEntityComponentHandle>("UsableComponent");
	defUsable.add_static_constant("EVENT_ON_USE",pragma::UsableComponent::EVENT_ON_USE);
	defUsable.add_static_constant("EVENT_CAN_USE",pragma::UsableComponent::EVENT_CAN_USE);
	gameMod[defUsable];

	auto defMap = luabind::class_<MapHandle,BaseEntityComponentHandle>("MapComponent");
	defMap.def("GetMapIndex",static_cast<void(*)(lua_State*,MapHandle&)>([](lua_State *l,MapHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,hComponent->GetMapIndex());
	}));
	gameMod[defMap];

	auto defSubmergible = luabind::class_<SubmergibleHandle,BaseEntityComponentHandle>("SubmergibleComponent");
	defSubmergible.def("IsSubmerged",&Lua::Submergible::IsSubmerged);
	defSubmergible.def("IsFullySubmerged",&Lua::Submergible::IsFullySubmerged);
	defSubmergible.def("GetSubmergedFraction",&Lua::Submergible::GetSubmergedFraction);
	defSubmergible.def("IsInWater",&Lua::Submergible::IsInWater);
	defSubmergible.def("GetSubmergedFractionProperty",static_cast<void(*)(lua_State*,SubmergibleHandle&)>([](lua_State *l,SubmergibleHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetSubmergedFractionProperty());
	}));
	defSubmergible.def("GetWaterEntity",static_cast<void(*)(lua_State*,SubmergibleHandle&)>([](lua_State *l,SubmergibleHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *entWater = hComponent->GetWaterEntity();
		if(entWater == nullptr)
			return;
		entWater->GetLuaObject()->push(l);
	}));
	defSubmergible.add_static_constant("EVENT_ON_WATER_SUBMERGED",pragma::SubmergibleComponent::EVENT_ON_WATER_SUBMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EMERGED",pragma::SubmergibleComponent::EVENT_ON_WATER_EMERGED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_ENTERED",pragma::SubmergibleComponent::EVENT_ON_WATER_ENTERED);
	defSubmergible.add_static_constant("EVENT_ON_WATER_EXITED",pragma::SubmergibleComponent::EVENT_ON_WATER_EXITED);
	gameMod[defSubmergible];

	auto defDamageable = luabind::class_<DamageableHandle,BaseEntityComponentHandle>("DamageableComponent");
	defDamageable.def("TakeDamage",&Lua::Damageable::TakeDamage);
	defDamageable.add_static_constant("EVENT_ON_TAKE_DAMAGE",pragma::DamageableComponent::EVENT_ON_TAKE_DAMAGE);
	gameMod[defDamageable];
}

void Lua::Velocity::GetVelocity(lua_State *l,VelocityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	luabind::object(l,hEnt->GetVelocity()).push(l);
}
void Lua::Velocity::SetVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetVelocity(vel);
}
void Lua::Velocity::AddVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->AddVelocity(vel);
}

void Lua::Velocity::GetAngularVelocity(lua_State *l,VelocityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	luabind::object(l,hEnt->GetAngularVelocity()).push(l);
}

void Lua::Velocity::SetAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetAngularVelocity(vel);
}

void Lua::Velocity::AddAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->AddAngularVelocity(vel);
}

void Lua::Velocity::GetLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	luabind::object(l,hEnt->GetLocalAngularVelocity()).push(l);
}
void Lua::Velocity::SetLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetLocalAngularVelocity(vel);
}
void Lua::Velocity::AddLocalAngularVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->AddLocalAngularVelocity(vel);
}

void Lua::Velocity::GetLocalVelocity(lua_State *l,VelocityHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	luabind::object(l,hEnt->GetLocalVelocity()).push(l);
}
void Lua::Velocity::SetLocalVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetLocalVelocity(vel);
}
void Lua::Velocity::AddLocalVelocity(lua_State *l,VelocityHandle &hEnt,Vector3 &vel)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->AddLocalVelocity(vel);
}

//////////////

void Lua::Damageable::TakeDamage(lua_State *l,DamageableHandle &hEnt,DamageInfo &info)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->TakeDamage(info);
}


//////////////

void Lua::Submergible::IsSubmerged(lua_State *l,SubmergibleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsSubmerged());
}
void Lua::Submergible::IsFullySubmerged(lua_State *l,SubmergibleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsFullySubmerged());
}
void Lua::Submergible::GetSubmergedFraction(lua_State *l,SubmergibleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushNumber(l,hEnt->GetSubmergedFraction());
}
void Lua::Submergible::IsInWater(lua_State *l,SubmergibleHandle &hEnt)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsInWater());
}

//////////////

void Lua::IK::SetIKControllerEnabled(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,bool b)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetIKControllerEnabled(ikControllerId,b);
}
void Lua::IK::IsIKControllerEnabled(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId)
{
	pragma::Lua::check_component(l,hEnt);
	Lua::PushBool(l,hEnt->IsIKControllerEnabled(ikControllerId));
}
void Lua::IK::SetIKEffectorPos(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,uint32_t effectorIdx,const Vector3 &pos)
{
	pragma::Lua::check_component(l,hEnt);
	hEnt->SetIKEffectorPos(ikControllerId,effectorIdx,pos);
}
void Lua::IK::GetIKEffectorPos(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,uint32_t effectorIdx)
{
	pragma::Lua::check_component(l,hEnt);
	auto *pos = hEnt->GetIKEffectorPos(ikControllerId,effectorIdx);
	if(pos == nullptr)
		return;
	Lua::Push<Vector3>(l,*pos);
}

//////////////

DLLNETWORK void Lua::TraceData::FillTraceResultTable(lua_State *l,TraceResult &res)
{
	Lua::Push<TraceResult>(l,res);
	//Lua::Push<boost::reference_wrapper<TraceResult>>(l,boost::ref<TraceResult>(res));

	// Deprecated
	/*auto tableIdx = Lua::CreateTable(l);

	Lua::PushString(l,"hit");
	Lua::PushBool(l,res.hit);
	Lua::SetTableValue(l,tableIdx);

	if(res.entity.IsValid())
	{
		Lua::PushString(l,"entity");
		lua_pushentity(l,res.entity);
		Lua::SetTableValue(l,tableIdx);
	}

	if(res.physObj.IsValid())
	{
		Lua::PushString(l,"physObj");
		luabind::object(l,res.physObj).push(l);
		Lua::SetTableValue(l,tableIdx);
	}

	if(res.collisionObj.IsValid())
	{
		Lua::PushString(l,"collisionObj");
		res.collisionObj->GetLuaObject()->push(l);
		Lua::SetTableValue(l,tableIdx);
	}

	Lua::PushString(l,"fraction");
	Lua::PushNumber(l,res.fraction);
	Lua::SetTableValue(l,tableIdx);

	Lua::PushString(l,"distance");
	Lua::PushNumber(l,res.distance);
	Lua::SetTableValue(l,tableIdx);

	Lua::PushString(l,"normal");
	Lua::Push<Vector3>(l,res.normal);
	Lua::SetTableValue(l,tableIdx);

	Lua::PushString(l,"position");
	Lua::Push<Vector3>(l,res.position);
	Lua::SetTableValue(l,tableIdx);
	//Lua::Push<TraceResult>(l,res);*/
}
#pragma optimize("",on)
