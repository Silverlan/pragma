/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lentity.h"
#include <pragma/math/angle/wvquaternion.h>
#include "luasystem.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include "pragma/lua/classes/ldef_model.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/ldef_color.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/audio/alsound.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/raytraces.h"
#include "pragma/audio/alsound_type.h"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/lua/lentity_type.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include "pragma/lua/lua_entity_component.hpp"
#include <sharedutils/datastream.h>
#include <pragma/physics/movetypes.h>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

bool Lua::is_entity(const luabind::object &o)
{
	return luabind::object_cast_nothrow<EntityHandle*>(o,static_cast<EntityHandle*>(nullptr));
}

void Lua::Entity::register_class(luabind::class_<EntityHandle> &classDef)
{
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid",&IsValid);
	classDef.def("Spawn",&Spawn);
	classDef.def("Remove",&Remove);
	classDef.def("GetIndex",&GetIndex);
	classDef.def("GetLocalIndex",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetLocalIndex());
	}));
	classDef.def("IsMapEntity",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->IsMapEntity());
	}));
	classDef.def("IsCharacter",&IsCharacter);
	classDef.def("IsPlayer",&IsPlayer);
	classDef.def("IsWorld",&IsWorld);
	classDef.def("IsInert",&IsInert);
	classDef.def("GetClass",&GetClass);
	//classDef.def("AddCallback",&AddCallback); // Obsolete
	classDef.def("IsScripted",&IsScripted);
	classDef.def("IsSpawned",&IsSpawned);
	classDef.def("SetKeyValue",&SetKeyValue);
	classDef.def("IsNPC",&IsNPC);
	classDef.def("IsWeapon",&IsWeapon);
	classDef.def("IsVehicle",&IsVehicle);
	classDef.def("RemoveSafely",&RemoveSafely);
	classDef.def("RemoveEntityOnRemoval",static_cast<void(*)(lua_State*,EntityHandle&,EntityHandle&)>(&RemoveEntityOnRemoval));
	classDef.def("RemoveEntityOnRemoval",static_cast<void(*)(lua_State*,EntityHandle&,EntityHandle&,Bool)>(&RemoveEntityOnRemoval));
	classDef.def("GetSpawnFlags",&GetSpawnFlags);
	classDef.def("GetPose",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<umath::ScaledTransform>(l,hEnt->GetPose());
	}));
	classDef.def("SetPose",static_cast<void(*)(lua_State*,EntityHandle&,const umath::Transform&)>([](lua_State *l,EntityHandle &hEnt,const umath::Transform &t) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		hEnt->SetPose(t);
	}));
	classDef.def("SetPose",static_cast<void(*)(lua_State*,EntityHandle&,const umath::ScaledTransform&)>([](lua_State *l,EntityHandle &hEnt,const umath::ScaledTransform &t) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		hEnt->SetPose(t);
	}));
	classDef.def("GetPos",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		if(!hEnt->GetTransformComponent())
		{
			Lua::Push<Vector3>(l,Vector3{});
			return;
		}
		Lua::Push<Vector3>(l,hEnt->GetPosition());
	}));
	classDef.def("SetPos",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &pos) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		hEnt->SetPosition(pos);
	}));
	classDef.def("GetAngles",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		if(!hEnt->GetTransformComponent())
		{
			Lua::Push<EulerAngles>(l,EulerAngles{});
			return;
		}
		Lua::Push<EulerAngles>(l,hEnt->GetTransformComponent()->GetAngles());
	}));
	classDef.def("SetAngles",static_cast<void(*)(lua_State*,EntityHandle&,const EulerAngles&)>([](lua_State *l,EntityHandle &hEnt,const EulerAngles &ang) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetAngles(ang);
	}));
	classDef.def("SetScale",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &v) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetScale(v);
	}));
	classDef.def("GetScale",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		if(!hEnt->GetTransformComponent())
		{
			Lua::Push<Vector3>(l,Vector3{1.f,1.f,1.f});
			return;
		}
		Lua::Push<Vector3>(l,hEnt->GetTransformComponent()->GetScale());
	}));
	classDef.def("GetRotation",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		if(!hEnt->GetTransformComponent())
		{
			Lua::Push<Quat>(l,Quat{});
			return;
		}
		Lua::Push<Quat>(l,hEnt->GetTransformComponent()->GetRotation());
	}));
	classDef.def("SetRotation",static_cast<void(*)(lua_State*,EntityHandle&,const Quat&)>([](lua_State *l,EntityHandle &hEnt,const Quat &rot) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *trComponent = static_cast<pragma::BaseTransformComponent*>(hEnt->AddComponent("transform").get());
		if(trComponent == nullptr)
			return;
		trComponent->SetRotation(rot);
	}));
	classDef.def("GetCenter",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		if(!hEnt->GetPhysicsComponent())
		{
			if(!hEnt->GetTransformComponent())
			{
				Lua::Push<Vector3>(l,Vector3{});
				return;
			}
			Lua::Push<Vector3>(l,hEnt->GetTransformComponent()->GetPosition());
			return;
		}
		Lua::Push<Vector3>(l,hEnt->GetPhysicsComponent()->GetCenter());
	}));

	classDef.def("AddComponent",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,bool)>([](lua_State *l,EntityHandle &hEnt,const std::string &name,bool bForceCreateNew) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->AddComponent(name,bForceCreateNew);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("AddComponent",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->AddComponent(name);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("AddComponent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,bool)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId,bool bForceCreateNew) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->AddComponent(componentId,bForceCreateNew);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("AddComponent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->AddComponent(componentId);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("RemoveComponent",static_cast<void(*)(lua_State*,EntityHandle&,BaseEntityComponentHandle&)>([](lua_State *l,EntityHandle &hEnt,BaseEntityComponentHandle &hComponent) {
		LUA_CHECK_ENTITY(l,hEnt);
		::pragma::Lua::check_component(l,hComponent);
		hEnt->RemoveComponent(*hComponent.get());
	}));
	classDef.def("RemoveComponent",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->RemoveComponent(name);
	}));
	classDef.def("RemoveComponent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->RemoveComponent(componentId);
	}));
	classDef.def("ClearComponents",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->ClearComponents();
	}));
	classDef.def("HasComponent",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *nw = engine->GetNetworkState(l);
		auto *game = nw->GetGameState();
		auto &componentManager = game->GetEntityComponentManager();
		auto componentId = pragma::INVALID_COMPONENT_ID;
		if(componentManager.GetComponentTypeId(name,componentId) == false)
		{
			Lua::PushBool(l,false);
			return;
		}
		Lua::PushBool(l,hEnt->HasComponent(componentId));
	}));
	classDef.def("HasComponent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->HasComponent(componentId));
	}));
	classDef.def("HasComponent",static_cast<void(*)(lua_State*,EntityHandle&,luabind::object)>([](lua_State *l,EntityHandle &hEnt,luabind::object) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,false);
	}));
	classDef.def("GetComponent",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->FindComponent(name);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("GetComponent",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pComponent = hEnt->FindComponent(componentId);
		if(pComponent.expired())
			return;
		pComponent->PushLuaObject(l);
	}));
	classDef.def("GetComponent",static_cast<void(*)(lua_State*,EntityHandle&,luabind::object)>([](lua_State *l,EntityHandle &hEnt,luabind::object) {
		LUA_CHECK_ENTITY(l,hEnt);
	}));
	if(Lua::get_extended_lua_modules_enabled())
	{
		// Shorthand functions
		classDef.def("C",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
			LUA_CHECK_ENTITY(l,hEnt);
			auto pComponent = hEnt->FindComponent(name);
			if(pComponent.expired())
				return;
			pComponent->PushLuaObject(l);
		}));
		classDef.def("C",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t componentId) {
			LUA_CHECK_ENTITY(l,hEnt);
			auto pComponent = hEnt->FindComponent(componentId);
			if(pComponent.expired())
				return;
			pComponent->PushLuaObject(l);
		}));
		classDef.def("C",static_cast<void(*)(lua_State*,EntityHandle&,luabind::object)>([](lua_State *l,EntityHandle &hEnt,luabind::object) {
			LUA_CHECK_ENTITY(l,hEnt);
		}));
	}
	classDef.def("GetComponents",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto &components = hEnt->GetComponents();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &pComponent : components)
		{
			Lua::PushInt(l,idx++);
			pComponent->PushLuaObject(l);
			Lua::SetTableValue(l,t);
		}
	}));
	classDef.def("GetTransformComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pTrComponent = hEnt->GetTransformComponent();
		if(pTrComponent == nullptr)
			return;
		pTrComponent->PushLuaObject(l);
	}));
	classDef.def("GetPhysicsComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pPhysComponent = hEnt->GetPhysicsComponent();
		if(pPhysComponent == nullptr)
			return;
		pPhysComponent->PushLuaObject(l);
	}));
	classDef.def("GetGenericComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pGenc = hEnt->GetGenericComponent();
		if(pGenc == nullptr)
			return;
		pGenc->PushLuaObject(l);
	}));
	classDef.def("GetCharacterComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pCharComponent = hEnt->GetCharacterComponent();
		if(pCharComponent.expired())
			return;
		pCharComponent->PushLuaObject(l);
	}));
	classDef.def("GetWeaponComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pWepComponent = hEnt->GetWeaponComponent();
		if(pWepComponent.expired())
			return;
		pWepComponent->PushLuaObject(l);
	}));
	classDef.def("GetVehicleComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pVhcComponent = hEnt->GetVehicleComponent();
		if(pVhcComponent.expired())
			return;
		pVhcComponent->PushLuaObject(l);
	}));
	classDef.def("GetPlayerComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pPlComponent = hEnt->GetPlayerComponent();
		if(pPlComponent.expired())
			return;
		pPlComponent->PushLuaObject(l);
	}));
	classDef.def("GetTimeScaleComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pTc = hEnt->GetTimeScaleComponent();
		if(pTc.expired())
			return;
		pTc->PushLuaObject(l);
	}));
	classDef.def("GetNameComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pnc = hEnt->GetNameComponent();
		if(pnc.expired())
			return;
		pnc->PushLuaObject(l);
	}));
	classDef.def("GetAIComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pAIComponent = hEnt->GetAIComponent();
		if(pAIComponent.expired())
			return;
		pAIComponent->PushLuaObject(l);
	}));
	classDef.def("GetModelComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pMdlComponent = hEnt->GetModelComponent();
		if(!pMdlComponent)
			return;
		pMdlComponent->PushLuaObject(l);
	}));
	classDef.def("GetAnimatedComponent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto pAnimComponent = hEnt->GetAnimatedComponent();
		if(pAnimComponent.expired())
			return;
		pAnimComponent->PushLuaObject(l);
	}));
	classDef.def("GetUuid",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushString(l,util::uuid_to_string(hEnt->GetUuid()));
	}));
	classDef.def("SetUuid",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &uuid) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetUuid(util::uuid_string_to_bytes(uuid));
	}));

	classDef.def("Save",&Save);
	classDef.def("Load",&Load);
	classDef.def("Copy",&Copy);

	classDef.def("GetAirDensity",&GetAirDensity);

	classDef.def("IsStatic",&IsStatic);
	classDef.def("IsDynamic",&IsDynamic);

	// Quick-access methods
	classDef.def("CreateSound",static_cast<void(*)(lua_State*,EntityHandle&,std::string,uint32_t)>([](lua_State *l,EntityHandle &hEnt,std::string sndname,uint32_t soundType) {
		LUA_CHECK_ENTITY(l,hEnt);
		std::shared_ptr<ALSound> snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType));
		if(snd == nullptr)
			return;
		luabind::object(l,snd).push(l);
	}));
	classDef.def("EmitSound",static_cast<void(*)(lua_State*,EntityHandle&,std::string,uint32_t,float,float)>([](lua_State *l,EntityHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch) {
		LUA_CHECK_ENTITY(l,hEnt);
		std::shared_ptr<ALSound> snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch);
		if(snd == nullptr)
			return;
		luabind::object(l,snd).push(l);
	}));
	classDef.def("EmitSound",static_cast<void(*)(lua_State*,EntityHandle&,std::string,uint32_t,float)>([](lua_State *l,EntityHandle &hEnt,std::string sndname,uint32_t soundType,float gain) {
		LUA_CHECK_ENTITY(l,hEnt);
		std::shared_ptr<ALSound> snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain);
		if(snd == nullptr)
			return;
		luabind::object(l,snd).push(l);
	}));
	classDef.def("EmitSound",static_cast<void(*)(lua_State*,EntityHandle&,std::string,uint32_t)>([](lua_State *l,EntityHandle &hEnt,std::string sndname,uint32_t soundType) {
		LUA_CHECK_ENTITY(l,hEnt);
		std::shared_ptr<ALSound> snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType));
		if(snd == nullptr)
			return;
		luabind::object(l,snd).push(l);
	}));
	classDef.def("GetName",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushString(l,hEnt->GetName());
	}));
	classDef.def("SetName",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetName(name);
	}));
	classDef.def("SetModel",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &mdl) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetModel(mdl);
	}));
	classDef.def("SetModel",static_cast<void(*)(lua_State*,EntityHandle&,const std::shared_ptr<Model>&)>([](lua_State *l,EntityHandle &hEnt,const std::shared_ptr<Model> &mdl) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetModel(mdl);
	}));
	classDef.def("ClearModel",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetModel(std::shared_ptr<Model>{nullptr});
	}));
	classDef.def("GetModel",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto mdl = hEnt->GetModel();
		if(mdl == nullptr)
			return;
		luabind::object(l,mdl).push(l);
	}));
	classDef.def("GetModelName",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushString(l,hEnt->GetModelName());
	}));
	classDef.def("GetAttachmentPose",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t attId) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto t = hEnt->GetAttachmentPose(attId);
		if(t.has_value() == false)
			return;
		Lua::Push<umath::Transform>(l,*t);
	}));
	classDef.def("GetSkin",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetSkin());
	}));
	classDef.def("SetSkin",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t skin) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetSkin(skin);
	}));
	classDef.def("GetBodyGroup",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &name) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetBodyGroup(name));
	}));
	classDef.def("GetBodyGroup",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t bgId) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto mdlC = hEnt->GetModelComponent();
		if(!mdlC)
			return;
		Lua::PushInt(l,mdlC->GetBodyGroup(bgId));
	}));
	classDef.def("SetBodyGroup",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,const std::string &group,uint32_t id) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetBodyGroup(group,id);
	}));
	classDef.def("SetBodyGroup",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t bgId,uint32_t id) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto mdlC = hEnt->GetModelComponent();
		if(!mdlC)
			return;
		mdlC->SetBodyGroup(bgId,id);
	}));
	classDef.def("GetParent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *parent = hEnt->GetParent();
		if(parent == nullptr)
			return;
		parent->PushLuaObject(l);
	}));
	classDef.def("ClearParent",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->ClearParent();
	}));
	classDef.def("SetParent",static_cast<void(*)(lua_State*,EntityHandle&,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt,EntityHandle &hParent) {
		LUA_CHECK_ENTITY(l,hEnt);
		LUA_CHECK_ENTITY(l,hParent);
		hEnt->SetParent(hParent.get());
	}));
	classDef.def("SetOwner",static_cast<void(*)(lua_State*,EntityHandle&,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt,EntityHandle &hOwner) {
		LUA_CHECK_ENTITY(l,hEnt);
		LUA_CHECK_ENTITY(l,hOwner);
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent*>(hEnt->AddComponent("ownable").get());
		if(ownableC == nullptr)
			return;
		ownableC->SetOwner(*hOwner.get());
	}));
	classDef.def("GetOwner",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *ownableC = dynamic_cast<pragma::BaseOwnableComponent*>(hEnt->FindComponent("ownable").get());
		if(ownableC == nullptr)
			return;
		auto *owner = ownableC->GetOwner();
		if(owner == nullptr)
			return;
		owner->GetLuaObject()->push(l);
	}));
	classDef.def("SetEnabled",&Lua::Entity::SetEnabled);
	classDef.def("SetTurnedOn",&Lua::Entity::SetEnabled);
	classDef.def("Enable",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {SetEnabled(l,hEnt,true);}));
	classDef.def("TurnOn",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {SetEnabled(l,hEnt,true);}));
	classDef.def("Disable",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {SetEnabled(l,hEnt,false);}));
	classDef.def("TurnOff",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {SetEnabled(l,hEnt,false);}));
	classDef.def("IsEnabled",&Lua::Entity::IsEnabled);
	classDef.def("IsTurnedOn",&Lua::Entity::IsEnabled);
	classDef.def("IsDisabled",&Lua::Entity::IsDisabled);
	classDef.def("IsTurnedOff",&Lua::Entity::IsDisabled);
	classDef.def("SetColor",&Lua::Entity::SetColor);
	classDef.def("GetColor",&Lua::Entity::GetColor);
	classDef.def("GetPhysicsObject",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *physObj = hEnt->GetPhysicsObject();
		if(physObj == nullptr)
			return;
		luabind::object(l,physObj->GetHandle()).push(l);
	}));
	classDef.def("InitializePhysics",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t type) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto *physObj = hEnt->InitializePhysics(static_cast<PHYSICSTYPE>(type));
		if(physObj == nullptr)
			return;
		luabind::object(l,physObj->GetHandle()).push(l);
	}));
	classDef.def("DestroyPhysicsObject",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->DestroyPhysicsObject();
	}));
	classDef.def("DropToFloor",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->DropToFloor();
	}));
	classDef.def("GetCollisionBounds",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto bounds = hEnt->GetCollisionBounds();
		Lua::Push<Vector3>(l,bounds.first);
		Lua::Push<Vector3>(l,bounds.second);
	}));
	classDef.def("SetCollisionFilterMask",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t mask) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetCollisionFilterMask(static_cast<CollisionMask>(mask));
	}));
	classDef.def("SetCollisionFilterGroup",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t group) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetCollisionFilterGroup(static_cast<CollisionMask>(group));
	}));
	classDef.def("GetCollisionFilterGroup",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,umath::to_integral(hEnt->GetCollisionFilterGroup()));
	}));
	classDef.def("GetCollisionFilterMask",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,umath::to_integral(hEnt->GetCollisionFilterMask()));
	}));
	classDef.def("GetForward",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<Vector3>(l,hEnt->GetForward());
	}));
	classDef.def("GetUp",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<Vector3>(l,hEnt->GetUp());
	}));
	classDef.def("GetRight",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<Vector3>(l,hEnt->GetRight());
	}));
	classDef.def("Input",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,EntityHandle&,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &input,EntityHandle &hActivator,EntityHandle &hCaller,const std::string &data) {
		LUA_CHECK_ENTITY(l,hEnt);
		LUA_CHECK_ENTITY(l,hActivator);
		LUA_CHECK_ENTITY(l,hCaller);
		hEnt->Input(input,hActivator.get(),hCaller.get(),data);
	}));
	classDef.def("Input",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,EntityHandle&,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt,const std::string &input,EntityHandle &hActivator,EntityHandle &hCaller) {
		LUA_CHECK_ENTITY(l,hEnt);
		LUA_CHECK_ENTITY(l,hActivator);
		LUA_CHECK_ENTITY(l,hCaller);
		hEnt->Input(input,hActivator.get(),hCaller.get());
	}));
	classDef.def("Input",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt,const std::string &input,EntityHandle &hActivator) {
		LUA_CHECK_ENTITY(l,hEnt);
		LUA_CHECK_ENTITY(l,hActivator);
		hEnt->Input(input,hActivator.get());
	}));
	classDef.def("Input",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &input) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->Input(input);
	}));
	classDef.def("GetHealth",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetHealth());
	}));
	classDef.def("GetMaxHealth",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetMaxHealth());
	}));
	classDef.def("SetHealth",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t health) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetHealth(health);
	}));
	classDef.def("SetMaxHealth",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t health) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetMaxHealth(health);
	}));
	classDef.def("SetVelocity",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &velocity) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetVelocity(velocity);
	}));
	classDef.def("AddVelocity",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &velocity) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->AddVelocity(velocity);
	}));
	classDef.def("SetAngularVelocity",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &velocity) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->SetAngularVelocity(velocity);
	}));
	classDef.def("AddAngularVelocity",static_cast<void(*)(lua_State*,EntityHandle&,const Vector3&)>([](lua_State *l,EntityHandle &hEnt,const Vector3 &velocity) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->AddAngularVelocity(velocity);
	}));
	classDef.def("GetVelocity",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<Vector3>(l,hEnt->GetVelocity());
	}));
	classDef.def("GetAngularVelocity",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::Push<Vector3>(l,hEnt->GetAngularVelocity());
	}));
	/*classDef.def("PlayAnimation",static_cast<void(*)(lua_State*,EntityHandle&,int32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->PlayAnimation(animation,static_cast<pragma::FPlayAnim>(flags));
	}));
	classDef.def("PlayLayeredAnimation",static_cast<void(*)(lua_State*,EntityHandle&,int32_t,int32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,int32_t slot,int32_t animation,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->PlayLayeredAnimation(slot,animation,static_cast<pragma::FPlayAnim>(flags));
	}));*/
	classDef.def("PlayActivity",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t activity,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayActivity(static_cast<Activity>(activity),static_cast<pragma::FPlayAnim>(flags)));
	}));
	classDef.def("PlayActivity",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t activity) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayActivity(static_cast<Activity>(activity),pragma::FPlayAnim::Default));
	}));
	classDef.def("PlayLayeredActivity",static_cast<void(*)(lua_State*,EntityHandle&,int32_t,uint32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,int32_t slot,uint32_t activity,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayLayeredActivity(slot,static_cast<Activity>(activity),static_cast<pragma::FPlayAnim>(flags)));
	}));
	classDef.def("PlayLayeredAnimation",static_cast<void(*)(lua_State*,EntityHandle&,int32_t,const std::string&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,int32_t slot,const std::string &anim,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayLayeredAnimation(slot,anim,static_cast<pragma::FPlayAnim>(flags)));
	}));
	classDef.def("StopLayeredAnimation",static_cast<void(*)(lua_State*,EntityHandle&,int32_t)>([](lua_State *l,EntityHandle &hEnt,int32_t slot) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->StopLayeredAnimation(slot);
	}));
	classDef.def("PlayAnimation",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t anim,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto animC = hEnt->GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim,static_cast<pragma::FPlayAnim>(flags));
	}));
	classDef.def("PlayAnimation",static_cast<void(*)(lua_State*,EntityHandle&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,uint32_t anim) {
		LUA_CHECK_ENTITY(l,hEnt);
		auto animC = hEnt->GetAnimatedComponent();
		if(animC.expired())
			return;
		animC->PlayAnimation(anim);
	}));
	classDef.def("PlayAnimation",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&,uint32_t)>([](lua_State *l,EntityHandle &hEnt,const std::string &anim,uint32_t flags) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayAnimation(anim,static_cast<pragma::FPlayAnim>(flags)));
	}));
	classDef.def("PlayAnimation",static_cast<void(*)(lua_State*,EntityHandle&,const std::string&)>([](lua_State *l,EntityHandle &hEnt,const std::string &anim) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushBool(l,hEnt->PlayAnimation(anim,pragma::FPlayAnim::Default));
	}));
	classDef.def("GetAnimation",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,hEnt->GetAnimation());
	}));
	classDef.def("GetActivity",static_cast<void(*)(lua_State*,EntityHandle&)>([](lua_State *l,EntityHandle &hEnt) {
		LUA_CHECK_ENTITY(l,hEnt);
		Lua::PushInt(l,umath::to_integral(hEnt->GetActivity()));
	}));
	classDef.def("TakeDamage",static_cast<void(*)(lua_State*,EntityHandle&,DamageInfo&)>([](lua_State *l,EntityHandle &hEnt,DamageInfo &dmgInfo) {
		LUA_CHECK_ENTITY(l,hEnt);
		hEnt->TakeDamage(dmgInfo);
	}));
	//

	// Enums
	classDef.add_static_constant("TYPE_DEFAULT",umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_LOCAL",umath::to_integral(LuaEntityType::Default));
	classDef.add_static_constant("TYPE_SHARED",umath::to_integral(LuaEntityType::Shared));

	classDef.add_static_constant("EVENT_HANDLE_KEY_VALUE",BaseEntity::EVENT_HANDLE_KEY_VALUE);
	classDef.add_static_constant("EVENT_ON_SPAWN",BaseEntity::EVENT_ON_SPAWN);
	classDef.add_static_constant("EVENT_ON_POST_SPAWN",BaseEntity::EVENT_ON_POST_SPAWN);
	classDef.add_static_constant("EVENT_ON_REMOVE",BaseEntity::EVENT_ON_REMOVE);

	classDef.add_static_constant("EVENT_ON_COMPONENT_ADDED",pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_ADDED);
	classDef.add_static_constant("EVENT_ON_COMPONENT_REMOVED",pragma::BaseEntityComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED);

	classDef.def(luabind::const_self ==luabind::const_self);
}

void Lua::Entity::IsValid(lua_State *l,EntityHandle &hEnt)
{
	lua_pushboolean(l,hEnt.IsValid() ? true : false);
}

void Lua::Entity::Remove(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->Remove();
}

void Lua::Entity::GetIndex(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,hEnt->GetIndex());
}

void Lua::Entity::IsCharacter(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,hEnt->IsCharacter());
}

void Lua::Entity::IsPlayer(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	lua_pushboolean(l,hEnt->IsPlayer());
}

void Lua::Entity::IsNPC(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	lua_pushboolean(l,hEnt->IsNPC());
}

void Lua::Entity::IsWorld(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	lua_pushboolean(l,hEnt->IsWorld());
}

void Lua::Entity::IsInert(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	lua_pushboolean(l,hEnt->IsInert());
}

void Lua::Entity::Spawn(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	if(hEnt->IsSpawned())
		return;
	hEnt->Spawn();
}

void Lua::Entity::GetClass(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	std::string classname = hEnt->GetClass();
	lua_pushstring(l,classname.c_str());
}

// Obsolete
/*void Lua::Entity::AddCallback(lua_State *l,EntityHandle &hEnt,std::string identifier,luabind::object o)
{
	luaL_checkfunction(l,3);
	LUA_CHECK_ENTITY(l,hEnt);
	auto callback = hEnt->AddLuaCallback(identifier,o);
	Lua::Push<CallbackHandle>(l,callback);
}
*/
void Lua::Entity::IsScripted(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	bool b = hEnt->IsScripted();
	lua_pushboolean(l,b);
}

void Lua::Entity::IsSpawned(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto b = hEnt->IsSpawned();
	lua_pushboolean(l,b);
}

void Lua::Entity::SetKeyValue(lua_State *l,EntityHandle &hEnt,std::string key,std::string val)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->SetKeyValue(key,val);
}

void Lua::Entity::IsWeapon(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	bool b = hEnt->IsWeapon();
	lua_pushboolean(l,b);
}

void Lua::Entity::IsVehicle(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	bool b = hEnt->IsVehicle();
	lua_pushboolean(l,b);
}
void Lua::Entity::RemoveSafely(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->RemoveSafely();
}
void Lua::Entity::RemoveEntityOnRemoval(lua_State *l,EntityHandle &hEnt,EntityHandle &hEntOther,Bool)
{
	LUA_CHECK_ENTITY(l,hEnt);
	LUA_CHECK_ENTITY(l,hEntOther);
	hEnt->RemoveEntityOnRemoval(hEntOther);
}
void Lua::Entity::RemoveEntityOnRemoval(lua_State *l,EntityHandle &hEnt,EntityHandle &hEntOther) {Lua::Entity::RemoveEntityOnRemoval(l,hEnt,hEntOther,true);}
void Lua::Entity::GetSpawnFlags(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushInt(l,hEnt->GetSpawnFlags());
}
void Lua::Entity::IsStatic(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,hEnt->IsStatic());
}
void Lua::Entity::IsDynamic(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushBool(l,hEnt->IsDynamic());
}

void Lua::Entity::SetEnabled(lua_State *l,EntityHandle &hEnt,bool enabled)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(hEnt->FindComponent("toggle").get());
	if(toggleC == nullptr && enabled == true)
		return;
	if(toggleC == nullptr)
		toggleC = dynamic_cast<pragma::BaseToggleComponent*>(hEnt->AddComponent("toggle").get());
	if(toggleC == nullptr)
		return;
	toggleC->SetTurnedOn(enabled);
}

void Lua::Entity::IsEnabled(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(hEnt->FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	Lua::PushBool(l,isEnabled);
}

void Lua::Entity::IsDisabled(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto isEnabled = true;
	auto *toggleC = dynamic_cast<pragma::BaseToggleComponent*>(hEnt->FindComponent("toggle").get());
	if(toggleC != nullptr)
		isEnabled = toggleC->IsTurnedOn();
	Lua::PushBool(l,!isEnabled);
}

Color Lua::Entity::GetColor(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY_RET(l,hEnt,Color::White);
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(hEnt->FindComponent("color").get());
	if(colorC == nullptr)
		return Color::White;
	return colorC->GetColor();
}
void Lua::Entity::SetColor(lua_State *l,EntityHandle &hEnt,const Color &color)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto *colorC = dynamic_cast<pragma::BaseColorComponent*>(hEnt->AddComponent("color").get());
	if(colorC == nullptr)
		return;
	colorC->SetColor(color);
}

void Lua::Entity::Save(lua_State *l,EntityHandle &hEnt,udm::LinkedPropertyWrapper &udm)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->Save(udm);
}
void Lua::Entity::Load(lua_State *l,EntityHandle &hEnt,udm::LinkedPropertyWrapper &udm)
{
	LUA_CHECK_ENTITY(l,hEnt);
	hEnt->Load(udm);
}
void Lua::Entity::Copy(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	auto *ent = hEnt->Copy();
	if(ent == nullptr)
		return;
	ent->GetLuaObject()->push(l);
}
void Lua::Entity::GetAirDensity(lua_State *l,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	Lua::PushNumber(l,1.225f); // Placeholder
}
