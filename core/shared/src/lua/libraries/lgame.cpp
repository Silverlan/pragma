/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/lua/libraries/lgame.h"
#include <sharedutils/functioncallback.h>
#include "pragma/lua/luacallback.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/baseworld.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/ai/navsystem.h"
#include <pragma/math/intersection.h>

extern DLLENGINE Engine *engine;
int Lua::game::add_callback(lua_State *l)
{
	std::string identifier = luaL_checkstring(l,1);
	luaL_checkfunction(l,2);
	NetworkState *state = engine->GetNetworkState(l);
	if(state == NULL)
		return 0;
	if(!state->IsGameActive())
		return 0;
	Game *game = state->GetGameState();
	CallbackHandle hCallback = game->AddLuaCallback(identifier,luabind::object(luabind::from_stack(l,2)));
	Lua::Push<CallbackHandle>(l,hCallback);
	return 1;
}

int Lua::game::call_callbacks(lua_State *l)
{
	auto bMultiReturn = false;
	int32_t argIdx = 1;
	if(Lua::IsBool(l,1))
		bMultiReturn = Lua::CheckBool(l,argIdx++);
	auto identifier = Lua::CheckString(l,argIdx);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *callbacks = game->GetLuaCallbacks(identifier);
	if(callbacks == nullptr)
		return 0;
	auto numArgs = Lua::GetStackTop(l) -argIdx;
	std::vector<int32_t> args;
	args.reserve(numArgs);
	for(auto i=decltype(numArgs){0};i<numArgs;++i)
		args.push_back(Lua::CreateReference(l,LUA_REGISTRYINDEX));
	//
	uint32_t numResults = 0;
	int32_t t = -1;
	uint32_t tIdx = 1;
	if(bMultiReturn == true)
		t = Lua::CreateTable(l);
	for(auto &hCallback : *callbacks)
	{
		if(hCallback.IsValid())
		{
			auto *callback = static_cast<LuaCallback*>(hCallback.get());
			auto &o = callback->GetLuaObject();
			auto top = Lua::GetStackTop(l);
			auto c = Lua::CallFunction(l,[&o,&args](lua_State *l) -> Lua::StatusCode {
				o.push(l);

				for(auto it=args.rbegin();it!=args.rend();++it)
					Lua::PushRegistryValue(l,*it);
				return Lua::StatusCode::Ok;
			},LUA_MULTRET);
			if(c == Lua::StatusCode::Ok)
			{
				numResults = Lua::GetStackTop(l) -top;
				if(numResults > 0) // Results will be forwarded to caller
				{
					if(t == -1)
						break;
					auto top = Lua::GetStackTop(l);
					auto tSub = Lua::CreateTable(l); /* 1 */
					uint32_t tSubIdx = 1;
					for(auto i=top -(static_cast<int32_t>(numResults) -1);i<=top;++i)
					{
						Lua::PushInt(l,tSubIdx++); /* 2 */
						Lua::PushValue(l,i); /* 3 */
						Lua::SetTableValue(l,tSub); /* 1 */
					}
					Lua::PushInt(l,tIdx++); /* 2 */
					Lua::PushValue(l,tSub); /* 3 */
					Lua::SetTableValue(l,t); /* 1 */
					Lua::Pop(l,1); /* 0 */

					Lua::Pop(l,numResults); // Don't need these anymore
				}
			}
		}
	}
	//
	for(auto arg : args)
		Lua::ReleaseReference(l,LUA_REGISTRYINDEX,arg);
	return (t == -1) ? numResults : 1;
}

int Lua::game::clear_callbacks(lua_State *l)
{
	auto identifier = Lua::CheckString(l,1);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *callbacks = game->GetLuaCallbacks(identifier);
	if(callbacks == nullptr)
		return 0;
	callbacks->clear();
	return 0;
}

int Lua::game::register_ammo_type(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto r = false;
	if(Lua::IsSet(l,2))
	{
		auto dmg = Lua::CheckInt(l,2);
		if(Lua::IsSet(l,3))
		{
			auto force = Lua::CheckNumber(l,3);
			if(Lua::IsSet(l,4))
			{
				auto dmgType = DAMAGETYPE(Lua::CheckInt(l,4));
				r = game->RegisterAmmoType(name,CInt32(dmg),CFloat(force),dmgType);
			}
			else
				r = game->RegisterAmmoType(name,CInt32(dmg),CFloat(force));
		}
		else
			r = game->RegisterAmmoType(name,CInt32(dmg));
	}
	else
		r = game->RegisterAmmoType(name);
	Lua::PushBool(l,r);
	return 1;
}
int Lua::game::get_ammo_type_id(lua_State *l)
{
	auto *name = Lua::CheckString(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	game->GetAmmoType(name,&ammoId);
	Lua::PushInt(l,ammoId);
	return 1;
}
int Lua::game::get_ammo_type_name(lua_State *l)
{
	auto typeId = Lua::CheckInt(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *type = game->GetAmmoType(CUInt32(typeId));
	if(type == nullptr)
		return 0;
	Lua::PushString(l,type->name);
	return 1;
}
int Lua::game::get_game_mode(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *o = game->GetGameModeLuaObject();
	if(o == nullptr)
		return 0;
	Lua::Push<luabind::object>(l,*o);
	return 1;
}
int Lua::game::get_light_color(lua_State *l)
{
	auto &pos = *Lua::CheckVector(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto componentIdLight = pragma::INVALID_COMPONENT_ID;
	auto componentIdToggle = pragma::INVALID_COMPONENT_ID;
	if(componentManager.GetComponentTypeId("light",componentIdLight) == false)
		return 0;
	componentManager.GetComponentTypeId("toggle",componentIdToggle);
	Vector3 col {};
	for(auto *ent : EntityIterator{*game,componentIdLight})
	{
		auto *pLightComponent = static_cast<pragma::BaseEnvLightComponent*>(ent->FindComponent(componentIdLight).get());
		auto *pToggleComponent = static_cast<pragma::BaseToggleComponent*>(ent->FindComponent(componentIdToggle).get());
		if(pLightComponent == nullptr || (pToggleComponent != nullptr && pToggleComponent->IsTurnedOn() == false))
			continue;
		auto pTrComponent = ent->GetTransformComponent();
		auto lightPos = pTrComponent.valid() ? pTrComponent->GetPosition() : Vector3{};
		auto pRadiusComponent = static_cast<pragma::BaseRadiusComponent*>(ent->FindComponent("radius").get());
		auto lightDist = (pRadiusComponent != nullptr) ? pRadiusComponent->GetRadius() : 0.f;
		auto dist = uvec::distance(pos,lightPos);
		if(dist >= lightDist)
			continue;
		auto pColComponent = static_cast<pragma::BaseColorComponent*>(ent->FindComponent("color").get());
		auto &lightCol = (pColComponent != nullptr) ? pColComponent->GetColor() : Color::White;
		auto brightness = lightCol.a /255.f;
		brightness *= dist /lightDist;
		col += Vector3(lightCol.r /255.f,lightCol.g /255.f,lightCol.b /255.f) *brightness;
	}
	Lua::Push<Vector3>(l,col);
	return 1;
}
int Lua::game::get_sound_intensity(lua_State *l)
{
	auto &pos = *Lua::CheckVector(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto &snds = nw->GetSounds();
	auto totalIntensity = 0.f;
	for(auto &rsnd : snds)
	{
		auto &snd = rsnd.get();
		if(snd.IsPlaying() == false)
			continue;
		totalIntensity += snd.GetSoundIntensity(pos);
	}
	Lua::PushNumber(l,totalIntensity);
	return 1;
}
int Lua::game::get_time_scale(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	Lua::PushNumber(l,game->GetTimeScale());
	return 1;
}
int Lua::game::set_time_scale(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto timeScale = Lua::CheckNumber(l,1);
	game->SetTimeScale(static_cast<float>(timeScale));
	return 0;
}
int Lua::game::is_game_mode_initialized(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	Lua::PushBool(l,game->IsGameModeInitialized());
	return 1;
}
std::pair<bool,int> Lua::game::load_map(lua_State *l,std::string &mapName,BaseEntity **ptrEntWorld,Vector3 &origin)
{
	mapName = Lua::CheckString(l,1);

	origin = {};
	if(Lua::IsSet(l,2))
		origin = *Lua::CheckVector(l,2);
	auto bNewWorld = false;
	if(Lua::IsSet(l,3))
		bNewWorld = true;
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	std::vector<EntityHandle> entities;
	auto r = false;
	if(bNewWorld == true)
	{
		r = game->LoadMap(mapName.c_str(),origin,&entities);
		if(entities.empty() == false && entities.front()->IsWorld())
			*ptrEntWorld = entities.front().get();
	}
	else
		r = game->LoadMap(mapName.c_str(),origin,&entities);
	Lua::PushBool(l,r);

	if(r == true)
	{
		auto t = Lua::CreateTable(l);
		uint32_t entIdx = 1;
		for(auto &hEnt : entities)
		{
			if(hEnt.IsValid() == false)
				continue;
			Lua::PushInt(l,entIdx++);
			lua_pushentity(l,hEnt.get());
			Lua::SetTableValue(l,t);
		}
		return {r,2};
	}
	return {r,1};
}
int Lua::game::get_nav_mesh(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &navMesh = game->GetNavMesh();
	if(navMesh == nullptr)
		return 0;
	Lua::Push<std::shared_ptr<pragma::nav::Mesh>>(l,navMesh);
	return 1;
}
int Lua::game::load_nav_mesh(lua_State *l)
{
	auto bReload = false;
	if(Lua::IsSet(l,1))
		bReload = Lua::CheckBool(l,1);

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto r = game->LoadNavMesh(bReload);
	Lua::PushBool(l,r);
	return 1;
}
int Lua::game::is_map_loaded(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	Lua::PushBool(l,game->IsMapLoaded());
	return 1;
}
int Lua::game::get_map_name(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	Lua::PushString(l,nw->GetMap());
	return 1;
}
int Lua::game::get_game_state_flags(lua_State *l)
{
	Lua::PushInt(l,umath::to_integral(engine->GetNetworkState(l)->GetGameState()->GetGameFlags()));
	return 1;
}
int Lua::game::raycast(lua_State *l)
{
	auto &data = *Lua::CheckTraceData(l,1);
	auto start = data.GetSourceOrigin();
	auto end = data.GetTargetOrigin();
	auto n = end -start;
	auto d = uvec::length(n);
	if(d > 0.f)
		n = n /d;

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	std::vector<BaseEntity*> *ents;
	game->GetEntities(&ents);

	//std::vector<TraceResult> res; // TODO
	auto bAllHits = data.HasFlag(RayCastFlags::ReportAllResults);
	BaseEntity *entClosest = nullptr;
	auto tClosest = 1.f;

	auto origin = data.GetSourceOrigin();
	auto btOrigin = origin;
	auto btEnd = data.GetTargetOrigin();
#ifdef _DEBUG
	//if(uvec::cmp(origin,data.GetTargetOrigin()) == true)
	//	btEnd.setX(btEnd.getX() +1.f); // Move it slightly, so source and target aren't the same (Causes fuzzyZero assertion error in debug mode)
#endif
	auto flags = data.GetFlags();
	auto mask = data.GetCollisionFilterMask();
	auto group = data.GetCollisionFilterGroup();
#ifdef ENABLE_DEPRECATED_PHYSICS
	auto *dataFilter = data.GetFilter();
	auto filter = (dataFilter != nullptr) ? std::shared_ptr<PhysClosestRayResultCallback>(new PhysClosestRayResultCallback(dataFilter->CreateClosestRayCallbackFilter(flags,group,mask,btOrigin,btEnd))) : nullptr;
#endif
	Intersection::LineMeshResult meshResult {};
	for(auto *ent : *ents)
	{
		if(ent == nullptr || ent->IsSpawned() == false)
			continue;
		auto pTrComponent = ent->GetTransformComponent();
		auto mdlComponent = ent->GetModelComponent();
		auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
		if(hMdl == nullptr || pTrComponent.expired())
			continue;
#ifdef ENABLE_DEPRECATED_PHYSICS
		if(dataFilter != nullptr && filter->ShouldPass(ent,nullptr,nullptr) == false)
			continue;
#endif
		Intersection::LineMesh(start,end -start,*hMdl,meshResult,true,pTrComponent->GetPosition(),pTrComponent->GetOrientation());
	}
	if(entClosest != nullptr)
	{
		TraceResult r {};
		r.fraction = tClosest;
		r.distance = d *tClosest;
		r.entity = entClosest->GetHandle();
		r.hitType = RayCastHitType::Block;
		r.position = start +n *(d *tClosest);
		r.startPosition = start;
		auto *phys = entClosest->GetPhysicsComponent()->GetPhysicsObject();
		if(phys != nullptr)
			r.physObj = phys->GetHandle();
		Lua::TraceData::FillTraceResultTable(l,r);
		return 1;
		//if(res.size() == res.capacity())
		//	res.reserve(res.size() +5);
		//res.push_back(r);
	}
	/*if(res.empty() == false)
	{
		auto t = Lua::CreateTable(l);
		auto idx = int32_t{};
		for(auto &r : res)
		{
			Lua::PushInt(l,idx);
			Lua_TraceData_FillTraceResultTable(l,r);
			Lua::SetTableValue(l,t);
		}
		return 1;
	}*/
	Lua::PushBool(l,false);
	return 1;
}
