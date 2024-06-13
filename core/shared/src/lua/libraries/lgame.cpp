/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/lua/luaapi.h"
#include "pragma/lua/types/udm.hpp"
#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "pragma/lua/libraries/lgame.h"
#include <sharedutils/functioncallback.h>
#include "pragma/lua/luacallback.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/lua_util_class.hpp"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/base_gamemode_component.hpp"
#include "pragma/entities/baseworld.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/alias_converter_t.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/game/value_driver.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/ai/navsystem.h"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <pragma/math/intersection.h>

extern DLLNETWORK Engine *engine;
Lua::opt<Lua::type<CallbackHandle>> Lua::game::add_callback(lua_State *l, const std::string &identifier, const func<void> &function)
{
	NetworkState *state = engine->GetNetworkState(l);
	if(state == NULL)
		return nil;
	if(!state->IsGameActive())
		return nil;
	Game *game = state->GetGameState();
	CallbackHandle hCallback = game->AddLuaCallback(identifier, function);
	return Lua::type<CallbackHandle> {l, hCallback};
}

static luabind::object call_callbacks(lua_State *l, Game &game, const std::string &identifier, const std::vector<luabind::object> &args)
{
	auto tReturn = luabind::newtable(l);
	uint32_t returnIdx = 1u;
	auto *callbacks = game.GetLuaCallbacks(identifier);
	if(!callbacks)
		return tReturn;
	for(auto &hCallback : *callbacks) {
		if(!hCallback.IsValid())
			continue;
		auto *callback = static_cast<LuaCallback *>(hCallback.get());
		auto &o = callback->GetLuaObject();
		auto top = Lua::GetStackTop(l);
		auto r = Lua::CallFunction(
		  l,
		  [&o, &args](lua_State *l) -> Lua::StatusCode {
			  o.push(l);

			  for(auto &arg : args)
				  arg.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  LUA_MULTRET);
		if(r == Lua::StatusCode::Ok) {
			auto numResults = Lua::GetStackTop(l) - top;
			if(numResults > 0) // Results will be forwarded to caller
			{
				top = Lua::GetStackTop(l);
				auto tReturnSub = luabind::newtable(l);
				for(auto i = decltype(numResults) {0u}; i < numResults; ++i)
					tReturnSub[i + 1] = luabind::object {luabind::from_stack(l, top - numResults + i + 1)};
				tReturn[returnIdx++] = tReturnSub;
				Lua::Pop(l, numResults);
			}
		}
	}
	return tReturn;
}

luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {}); }
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0}); }
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1}); }
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2}); }
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3});
}
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3, arg4});
}
luabind::object Lua::game::call_callbacks(lua_State *l, Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4, luabind::object arg5)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3, arg4, arg5});
}

void Lua::game::clear_callbacks(lua_State *l, const std::string &identifier)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *callbacks = game->GetLuaCallbacks(identifier);
	if(callbacks == nullptr)
		return;
	callbacks->clear();
}
bool Lua::game::register_ammo_type(lua_State *l, const std::string &name, int32_t damage, float force, DAMAGETYPE damageType)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->RegisterAmmoType(name, damage, force, damageType);
}
Lua::opt<uint32_t> Lua::game::get_ammo_type_id(lua_State *l, const std::string &name)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	if(game->GetAmmoType(name, &ammoId) == nullptr)
		return nil;
	return {l, ammoId};
}
Lua::opt<std::string> Lua::game::get_ammo_type_name(lua_State *l, uint32_t typeId)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *type = game->GetAmmoType(CUInt32(typeId));
	if(type == nullptr)
		return nil;
	return {l, type->name};
}
Lua::opt<Lua::type<pragma::BaseGamemodeComponent>> Lua::game::get_game_mode(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *ent = game->GetGameModeEntity();
	if(ent == nullptr)
		return nil;
	auto gmC = static_cast<pragma::BaseGamemodeComponent *>(ent->FindComponent("gamemode").get());
	if(gmC == nullptr)
		return nil;
	return gmC->GetLuaObject();
}
Lua::opt<Vector3> Lua::game::get_light_color(lua_State *l, const Vector3 &pos)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto componentIdLight = pragma::INVALID_COMPONENT_ID;
	auto componentIdToggle = pragma::INVALID_COMPONENT_ID;
	if(componentManager.GetComponentTypeId("light", componentIdLight) == false)
		return nil;
	componentManager.GetComponentTypeId("toggle", componentIdToggle);
	Vector3 col {};
	for(auto *ent : EntityIterator {*game, componentIdLight}) {
		auto *pLightComponent = static_cast<pragma::BaseEnvLightComponent *>(ent->FindComponent(componentIdLight).get());
		auto *pToggleComponent = static_cast<pragma::BaseToggleComponent *>(ent->FindComponent(componentIdToggle).get());
		if(pLightComponent == nullptr || (pToggleComponent != nullptr && pToggleComponent->IsTurnedOn() == false))
			continue;
		auto pTrComponent = ent->GetTransformComponent();
		auto lightPos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};
		auto pRadiusComponent = static_cast<pragma::BaseRadiusComponent *>(ent->FindComponent("radius").get());
		auto lightDist = (pRadiusComponent != nullptr) ? pRadiusComponent->GetRadius() : 0.f;
		auto dist = uvec::distance(pos, lightPos);
		if(dist >= lightDist)
			continue;
		auto pColComponent = static_cast<pragma::BaseColorComponent *>(ent->FindComponent("color").get());
		auto &lightCol = (pColComponent != nullptr) ? pColComponent->GetColor() : Color::White;
		auto brightness = lightCol.a / 255.f;
		brightness *= dist / lightDist;
		col += Vector3(lightCol.r / 255.f, lightCol.g / 255.f, lightCol.b / 255.f) * brightness;
	}
	return {l, col};
}
float Lua::game::get_sound_intensity(lua_State *l, const Vector3 &pos)
{
	auto *nw = engine->GetNetworkState(l);
	auto &snds = nw->GetSounds();
	auto totalIntensity = 0.f;
	for(auto &rsnd : snds) {
		auto &snd = rsnd.get();
		if(snd.IsPlaying() == false)
			continue;
		totalIntensity += snd.GetSoundIntensity(pos);
	}
	return totalIntensity;
}
float Lua::game::get_time_scale(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->GetTimeScale();
}
void Lua::game::set_time_scale(lua_State *l, float timeScale)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	game->SetTimeScale(timeScale);
}
bool Lua::game::is_game_mode_initialized(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->IsGameModeInitialized();
}
std::pair<bool, int> Lua::game::load_map(lua_State *l, std::string &mapName, BaseEntity **ptrEntWorld, Vector3 &origin)
{
	mapName = Lua::CheckString(l, 1);

	origin = {};
	if(Lua::IsSet(l, 2))
		origin = *Lua::CheckVector(l, 2);
	auto bNewWorld = false;
	if(Lua::IsSet(l, 3))
		bNewWorld = true;
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	std::vector<EntityHandle> entities;
	auto r = false;
	if(bNewWorld == true) {
		r = game->LoadMap(mapName.c_str(), origin, &entities);
		if(entities.empty() == false && entities.front()->IsWorld())
			*ptrEntWorld = entities.front().get();
	}
	else
		r = game->LoadMap(mapName.c_str(), origin, &entities);
	Lua::PushBool(l, r);

	if(r == true) {
		auto t = Lua::CreateTable(l);
		uint32_t entIdx = 1;
		for(auto &hEnt : entities) {
			if(hEnt.valid() == false)
				continue;
			Lua::PushInt(l, entIdx++);
			lua_pushentity(l, hEnt.get());
			Lua::SetTableValue(l, t);
		}
		return {r, 2};
	}
	return {r, 1};
}
Lua::opt<std::shared_ptr<pragma::nav::Mesh>> Lua::game::get_nav_mesh(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &navMesh = game->GetNavMesh();
	if(navMesh == nullptr)
		return nil;
	return {l, navMesh};
}
bool Lua::game::load_nav_mesh(lua_State *l, bool reload)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->LoadNavMesh(reload);
}
bool Lua::game::is_map_loaded(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->IsMapLoaded();
}
std::string Lua::game::get_map_name(lua_State *l)
{
	auto *nw = engine->GetNetworkState(l);
	return nw->GetMap();
}
Game::GameFlags Lua::game::get_game_state_flags(lua_State *l) { return engine->GetNetworkState(l)->GetGameState()->GetGameFlags(); }
bool Lua::game::raycast(lua_State *l, const ::TraceData &data)
{
	auto start = data.GetSourceOrigin();
	auto end = data.GetTargetOrigin();
	auto n = end - start;
	auto d = uvec::length(n);
	if(d > 0.f)
		n = n / d;

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();
	std::vector<BaseEntity *> *ents;
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
	auto filter = (dataFilter != nullptr) ? std::shared_ptr<PhysClosestRayResultCallback>(new PhysClosestRayResultCallback(dataFilter->CreateClosestRayCallbackFilter(flags, group, mask, btOrigin, btEnd))) : nullptr;
#endif
	Intersection::LineMeshResult meshResult {};
	for(auto *ent : *ents) {
		if(ent == nullptr || ent->IsSpawned() == false)
			continue;
		auto pTrComponent = ent->GetTransformComponent();
		auto &hMdl = ent->GetModel();
		if(hMdl == nullptr || pTrComponent == nullptr)
			continue;
#ifdef ENABLE_DEPRECATED_PHYSICS
		if(dataFilter != nullptr && filter->ShouldPass(ent, nullptr, nullptr) == false)
			continue;
#endif
		Intersection::LineMesh(start, end - start, *hMdl, meshResult, true, pTrComponent->GetPosition(), pTrComponent->GetRotation());
	}
#if 0
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
#endif
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
	return false;
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, ValueDriverDescriptor);
#endif
void Lua::game::register_shared_functions(lua_State *l, luabind::module_ &modGame)
{
	modGame[luabind::def("add_callback", Lua::game::add_callback),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &)>(Lua::game::call_callbacks)), luabind::def("clear_callbacks", Lua::game::clear_callbacks),

	  // Aliases
	  luabind::def("add_event_listener", Lua::game::add_callback),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &, luabind::object)>(Lua::game::call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua_State *, Game &, const std::string &)>(Lua::game::call_callbacks)), luabind::def("clear_event_listeners", Lua::game::clear_callbacks),

	  luabind::def("register_ammo_type", Lua::game::register_ammo_type),
	  luabind::def(
	    "register_ammo_type", +[](lua_State *l, const std::string &name, int32_t damage, float force) { return Lua::game::register_ammo_type(l, name, damage, force); }),
	  luabind::def(
	    "register_ammo_type", +[](lua_State *l, const std::string &name, int32_t damage) { return Lua::game::register_ammo_type(l, name, damage); }),
	  luabind::def(
	    "register_ammo_type", +[](lua_State *l, const std::string &name) { return Lua::game::register_ammo_type(l, name); }),
	  luabind::def("get_ammo_type_id", Lua::game::get_ammo_type_id), luabind::def("get_ammo_type_name", Lua::game::get_ammo_type_name), luabind::def("get_game_mode", Lua::game::get_game_mode), luabind::def("get_light_color", Lua::game::get_light_color),
	  luabind::def("get_sound_intensity", Lua::game::get_sound_intensity), luabind::def("get_time_scale", Lua::game::get_time_scale), luabind::def("is_game_mode_initialized", Lua::game::is_game_mode_initialized),
	  // luabind::def("raycast",Lua::game::raycast),
	  luabind::def("get_nav_mesh", Lua::game::get_nav_mesh), luabind::def("load_nav_mesh", Lua::game::load_nav_mesh),
	  luabind::def(
	    "load_nav_mesh", +[](lua_State *l) { return Lua::game::load_nav_mesh(l); }),
	  luabind::def("is_map_loaded", Lua::game::is_map_loaded), luabind::def("get_map_name", Lua::game::get_map_name), luabind::def("is_game_initialized", &Game::IsGameInitialized), luabind::def("is_game_ready", &Game::IsGameReady),
	  luabind::def("is_map_initialized", &Game::IsMapInitialized), luabind::def("get_game_state_flags", Lua::game::get_game_state_flags),
	  luabind::def(
	    "update_animations", +[](Game &game, float dt) { game.UpdateAnimations(dt); })];

	auto classDefDescriptor = pragma::lua::register_class<pragma::ValueDriverDescriptor>(l, "ValueDriverDescriptor");
	classDefDescriptor->def(luabind::constructor<lua_State *, std::string, std::unordered_map<std::string, std::string>, std::unordered_map<std::string, udm::PProperty>>());
	classDefDescriptor->def(luabind::constructor<lua_State *, std::string, std::unordered_map<std::string, std::string>>());
	classDefDescriptor->def(luabind::constructor<lua_State *, std::string>());
	classDefDescriptor->property("expression", static_cast<std::string (*)(lua_State *, pragma::ValueDriverDescriptor &)>([](lua_State *l, pragma::ValueDriverDescriptor &descriptor) -> std::string { return descriptor.GetExpression(); }),
	  static_cast<void (*)(lua_State *, pragma::ValueDriverDescriptor &, const std::string &)>([](lua_State *l, pragma::ValueDriverDescriptor &descriptor, const std::string &expr) { descriptor.SetExpression(expr); }));
	classDefDescriptor->def("AddReference", &pragma::ValueDriverDescriptor::AddReference);
	classDefDescriptor->def("AddConstant", static_cast<void (*)(pragma::ValueDriverDescriptor &, const std::string &, udm::PProperty)>([](pragma::ValueDriverDescriptor &descriptor, const std::string &name, udm::PProperty prop) { descriptor.AddConstant(name, prop); }));
	classDefDescriptor->def("AddConstant", static_cast<void (*)(pragma::ValueDriverDescriptor &, const std::string &, const Lua::classObject &)>([](pragma::ValueDriverDescriptor &descriptor, const std::string &name, const Lua::classObject &udmType) {
		for(auto type : udm::GENERIC_TYPES) {
			auto r = udm::visit<false, true, false>(type, [&udmType, &descriptor, &name](auto tag) mutable -> bool {
				using T = typename decltype(tag)::type;
				auto *o = luabind::object_cast_nothrow<T *>(udmType, luabind::pointer_policy<0> {}, static_cast<T *>(nullptr));
				if(o) {
					auto cpy = *o;
					descriptor.AddConstant<T>(name, std::move(cpy));
					return true;
				}
				return false;
			});
			if(r)
				break;
		}
	}));
	classDefDescriptor->def("GetConstants", &pragma::ValueDriverDescriptor::GetConstants);
	classDefDescriptor->def("GetReferences", &pragma::ValueDriverDescriptor::GetReferences);
	modGame[*classDefDescriptor];
}
