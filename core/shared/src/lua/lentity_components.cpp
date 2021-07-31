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
#include "pragma/lua/classes/entity_components.hpp"
#include <pragma/physics/movetypes.h>
#pragma optimize("",off)
namespace Lua
{
	namespace IK
	{
		static void GetIKEffectorPos(lua_State *l,IKHandle &hEnt,uint32_t ikControllerId,uint32_t effectorIdx);
	};
};

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
	defVelocity.def("GetVelocityProperty",static_cast<void(*)(lua_State*,VelocityHandle&)>([](lua_State *l,VelocityHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetVelocityProperty());
	}));
	defVelocity.def("GetAngularVelocityProperty",static_cast<void(*)(lua_State*,VelocityHandle&)>([](lua_State *l,VelocityHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetAngularVelocityProperty());
	}));
	gameMod[defVelocity];

	auto defGlobal = luabind::class_<pragma::GlobalNameComponent,pragma::BaseEntityComponent>("GlobalComponent");
	defGlobal.def("GetGlobalName",&pragma::GlobalNameComponent::GetGlobalName);
	defGlobal.def("SetGlobalName",&pragma::GlobalNameComponent::SetGlobalName);
	gameMod[defGlobal];

	auto defComposite = luabind::class_<pragma::CompositeComponent,pragma::BaseEntityComponent>("CompositeComponent");
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
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
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
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
		}
		return tEnts;
	}));
	defComposite.def("GetRootGroup",static_cast<pragma::CompositeGroup&(*)(lua_State*,CompositeHandle&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,const std::string &name) -> pragma::CompositeGroup& {
		pragma::Lua::check_component(l,hComponent);
		return hComponent->GetRootCompositeGroup();
	}));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,CompositeHandle&,BaseEntity&)>([](lua_State *l,CompositeHandle &hComponent,BaseEntity &ent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->GetRootCompositeGroup().AddEntity(ent);
	}));
	defComposite.def("AddEntity",static_cast<void(*)(lua_State*,CompositeHandle&,BaseEntity&,const std::string&)>([](lua_State *l,CompositeHandle &hComponent,BaseEntity &ent,const std::string &groupName) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->GetRootCompositeGroup().AddChildGroup(groupName).AddEntity(ent);
	}));
	auto defCompositeGroup = luabind::class_<pragma::CompositeGroup>("CompositeGroup");
	defCompositeGroup.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,BaseEntity&)>([](lua_State *l,pragma::CompositeGroup &hComponent,BaseEntity &ent) {
		hComponent.AddEntity(ent);
	}));
	defCompositeGroup.def("AddEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,BaseEntity&,const std::string&)>([](lua_State *l,pragma::CompositeGroup &hComponent,BaseEntity &ent,const std::string &groupName) {
		hComponent.AddChildGroup(groupName).AddEntity(ent);
	}));
	defCompositeGroup.def("RemoveEntity",static_cast<void(*)(lua_State*,pragma::CompositeGroup&,BaseEntity&)>([](lua_State *l,pragma::CompositeGroup &hComponent,BaseEntity &ent) {
		hComponent.RemoveEntity(ent);
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
			if(!hEnt.valid())
				continue;
			tEnts[idx++] = hEnt.get()->GetLuaObject();
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
	
	auto defAnimated2 = luabind::class_<pragma::Animated2Component,pragma::BaseEntityComponent>("Animated2Component");
	defAnimated2.def("SetPlaybackRate",&pragma::Animated2Component::SetPlaybackRate);
	defAnimated2.def("GetPlaybackRate",&pragma::Animated2Component::GetPlaybackRate);
	defAnimated2.def("GetPlaybackRateProperty",&pragma::Animated2Component::GetPlaybackRateProperty,luabind::property_policy<0>{});
	defAnimated2.def("ClearAnimationManagers",static_cast<void(*)(lua_State*,Animated2Handle&)>([](lua_State *l,Animated2Handle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->ClearAnimationManagers();
	}));
	defAnimated2.def("AddAnimationManager",&pragma::Animated2Component::AddAnimationManager);
	defAnimated2.def("RemoveAnimationManager",&pragma::Animated2Component::RemoveAnimationManager);
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

	auto defIK = luabind::class_<pragma::IKComponent,pragma::BaseEntityComponent>("IKComponent");
	defIK.def("SetIKControllerEnabled",&pragma::IKComponent::SetIKControllerEnabled);
	defIK.def("IsIKControllerEnabled",&pragma::IKComponent::IsIKControllerEnabled);
	defIK.def("SetIKEffectorPos",&pragma::IKComponent::SetIKEffectorPos);
	defIK.def("GetIKEffectorPos",&Lua::IK::GetIKEffectorPos);
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
	defSubmergible.def("GetSubmergedFractionProperty",static_cast<void(*)(lua_State*,SubmergibleHandle&)>([](lua_State *l,SubmergibleHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::Property::push(l,*hComponent->GetSubmergedFractionProperty());
	}));
	defSubmergible.def("GetWaterEntity",static_cast<void(*)(lua_State*,SubmergibleHandle&)>([](lua_State *l,SubmergibleHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		auto *entWater = hComponent->GetWaterEntity();
		if(entWater == nullptr)
			return;
		entWater->GetLuaObject().push(l);
	}));
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
