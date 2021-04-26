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
#include "pragma/util/util_ballistic.h"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/lua/lentity_components_base_types.hpp"
#include <pragma/physics/movetypes.h>

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

void Game::RegisterLuaEntityComponents(luabind::module_ &gameMod)
{
	auto def = luabind::class_<BaseEntityComponentHandle>("EntityComponent");
	RegisterLuaEntityComponent(def);
	gameMod[def];

	Lua::register_gravity_component(gameMod);

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
	
	auto defAnimated2 = luabind::class_<Animated2Handle,BaseEntityComponentHandle>("Animated2Component");
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
