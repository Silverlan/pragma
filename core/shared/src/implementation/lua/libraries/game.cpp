// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"
#include <cassert>

module pragma.shared;

import :scripting.lua.libraries.game;
import :game.value_driver;

Lua::opt<Lua::type<CallbackHandle>> Lua::game::add_callback(lua::State *l, const std::string &identifier, const func<void> &function)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return nil;
	if(!state->IsGameActive())
		return nil;
	pragma::Game *game = state->GetGameState();
	CallbackHandle hCallback = game->AddLuaCallback(identifier, function);
	return Lua::type<CallbackHandle> {l, hCallback};
}

static luabind::object call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, const std::vector<luabind::object> &args)
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
		  [&o, &args](lua::State *l) -> Lua::StatusCode {
			  o.push(l);

			  for(auto &arg : args)
				  arg.push(l);
			  return Lua::StatusCode::Ok;
		  },
		  lua::MultiReturn);
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

luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {}); }
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0}); }
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1}); }
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2) { return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2}); }
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3});
}
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3, arg4});
}
luabind::object Lua::game::call_callbacks(lua::State *l, pragma::Game &game, const std::string &identifier, luabind::object arg0, luabind::object arg1, luabind::object arg2, luabind::object arg3, luabind::object arg4, luabind::object arg5)
{
	return ::call_callbacks(l, game, identifier, std::vector<luabind::object> {arg0, arg1, arg2, arg3, arg4, arg5});
}

void Lua::game::clear_callbacks(lua::State *l, const std::string &identifier)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *callbacks = game->GetLuaCallbacks(identifier);
	if(callbacks == nullptr)
		return;
	callbacks->clear();
}
bool Lua::game::register_ammo_type(lua::State *l, const std::string &name, int32_t damage, float force, DamageType damageType)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->RegisterAmmoType(name, damage, force, damageType);
}
Lua::opt<uint32_t> Lua::game::get_ammo_type_id(lua::State *l, const std::string &name)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	UInt32 ammoId = 0;
	if(game->GetAmmoType(name, &ammoId) == nullptr)
		return nil;
	return {l, ammoId};
}
Lua::opt<std::string> Lua::game::get_ammo_type_name(lua::State *l, uint32_t typeId)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *type = game->GetAmmoType(CUInt32(typeId));
	if(type == nullptr)
		return nil;
	return {l, type->name};
}
Lua::opt<Lua::type<pragma::BaseGamemodeComponent>> Lua::game::get_game_mode(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto *ent = game->GetGameModeEntity();
	if(ent == nullptr)
		return nil;
	auto gmC = static_cast<pragma::BaseGamemodeComponent *>(ent->FindComponent("gamemode").get());
	if(gmC == nullptr)
		return nil;
	return gmC->GetLuaObject();
}
Lua::opt<Vector3> Lua::game::get_light_color(lua::State *l, const Vector3 &pos)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto componentIdLight = pragma::INVALID_COMPONENT_ID;
	auto componentIdToggle = pragma::INVALID_COMPONENT_ID;
	if(componentManager.GetComponentTypeId("light", componentIdLight) == false)
		return nil;
	componentManager.GetComponentTypeId("toggle", componentIdToggle);
	Vector3 col {};
	for(auto *ent : pragma::ecs::EntityIterator {*game, componentIdLight}) {
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
		auto &lightCol = (pColComponent != nullptr) ? pColComponent->GetColor() : colors::White;
		auto brightness = lightCol.a / 255.f;
		brightness *= dist / lightDist;
		col += Vector3(lightCol.r / 255.f, lightCol.g / 255.f, lightCol.b / 255.f) * brightness;
	}
	return {l, col};
}
float Lua::game::get_sound_intensity(lua::State *l, const Vector3 &pos)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
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
float Lua::game::get_time_scale(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->GetTimeScale();
}
void Lua::game::set_time_scale(lua::State *l, float timeScale)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	game->SetTimeScale(timeScale);
}
bool Lua::game::is_game_mode_initialized(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->IsGameModeInitialized();
}
std::pair<bool, int> Lua::game::load_map(lua::State *l, std::string &mapName, pragma::ecs::BaseEntity **ptrEntWorld, Vector3 &origin)
{
	mapName = CheckString(l, 1);

	origin = {};
	if(IsSet(l, 2))
		origin = Lua::Check<Vector3>(l, 2);
	auto bNewWorld = false;
	if(IsSet(l, 3))
		bNewWorld = true;
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
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
	PushBool(l, r);

	if(r == true) {
		auto t = CreateTable(l);
		uint32_t entIdx = 1;
		for(auto &hEnt : entities) {
			if(hEnt.valid() == false)
				continue;
			PushInt(l, entIdx++);
			hEnt.get()->GetLuaObject().push(l);
			SetTableValue(l, t);
		}
		return {r, 2};
	}
	return {r, 1};
}
Lua::opt<std::shared_ptr<pragma::nav::Mesh>> Lua::game::get_nav_mesh(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	auto &navMesh = game->GetNavMesh();
	if(navMesh == nullptr)
		return nil;
	return {l, navMesh};
}
bool Lua::game::load_nav_mesh(lua::State *l, bool reload)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->LoadNavMesh(reload);
}
bool Lua::game::is_map_loaded(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	return game->IsMapLoaded();
}
std::string Lua::game::get_map_name(lua::State *l)
{
	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	return nw->GetMap();
}
pragma::Game::GameFlags Lua::game::get_game_state_flags(lua::State *l) { return pragma::Engine::Get()->GetNetworkState(l)->GetGameState()->GetGameFlags(); }
bool Lua::game::raycast(lua::State *l, const pragma::physics::TraceData &data)
{
	auto start = data.GetSourceOrigin();
	auto end = data.GetTargetOrigin();
	auto n = end - start;
	auto d = uvec::length(n);
	if(d > 0.f)
		n = n / d;

	auto *nw = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = nw->GetGameState();
	std::vector<pragma::ecs::BaseEntity *> *ents;
	game->GetEntities(&ents);

	//std::vector<TraceResult> res; // TODO
	auto bAllHits = data.HasFlag(pragma::physics::RayCastFlags::ReportAllResults);
	pragma::ecs::BaseEntity *entClosest = nullptr;
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
	pragma::math::intersection::LineMeshResult meshResult {};
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
		pragma::math::intersection::line_with_mesh(start, end - start, *hMdl, meshResult, true, pTrComponent->GetPosition(), pTrComponent->GetRotation());
	}
#if 0
	if(entClosest != nullptr)
	{
		TraceResult r {};
		r.fraction = tClosest;
		r.distance = d *tClosest;
		r.entity = entClosest->GetHandle();
		r.hitType = pragma::physics::RayCastHitType::Block;
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

void Lua::game::register_shared_functions(lua::State *l, luabind::module_ &modGame)
{
	modGame[(luabind::def("add_callback", add_callback),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object)>(call_callbacks)),
	  luabind::def("call_callbacks", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &)>(call_callbacks)), luabind::def("clear_callbacks", clear_callbacks),

	  // Aliases
	  luabind::def("add_event_listener", add_callback),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &, luabind::object)>(call_callbacks)),
	  luabind::def("call_event_listeners", static_cast<luabind::object (*)(lua::State *, pragma::Game &, const std::string &)>(call_callbacks)), luabind::def("clear_event_listeners", clear_callbacks),

	  luabind::def("register_ammo_type", register_ammo_type),
	  luabind::def(
	    "register_ammo_type", +[](lua::State *l, const std::string &name, int32_t damage, float force) { return register_ammo_type(l, name, damage, force); }),
	  luabind::def(
	    "register_ammo_type", +[](lua::State *l, const std::string &name, int32_t damage) { return register_ammo_type(l, name, damage); }),
	  luabind::def(
	    "register_ammo_type", +[](lua::State *l, const std::string &name) { return register_ammo_type(l, name); }),
	  luabind::def("get_ammo_type_id", get_ammo_type_id), luabind::def("get_ammo_type_name", get_ammo_type_name), luabind::def("get_game_mode", get_game_mode), luabind::def("get_light_color", get_light_color),
	  luabind::def("get_sound_intensity", get_sound_intensity), luabind::def("get_time_scale", get_time_scale), luabind::def("is_game_mode_initialized", is_game_mode_initialized),
	  // luabind::def("raycast",Lua::game::raycast),
	  luabind::def("get_nav_mesh", get_nav_mesh), luabind::def("load_nav_mesh", load_nav_mesh),
	  luabind::def(
	    "load_nav_mesh", +[](lua::State *l) { return load_nav_mesh(l); }),
	  luabind::def("is_map_loaded", is_map_loaded), luabind::def("get_map_name", get_map_name), luabind::def("is_game_initialized", &pragma::Game::IsGameInitialized), luabind::def("is_game_ready", &pragma::Game::IsGameReady),
	  luabind::def("is_map_initialized", &pragma::Game::IsMapInitialized), luabind::def("get_game_state_flags", get_game_state_flags), luabind::def("update_animations", +[](pragma::Game &game, float dt) { game.UpdateAnimations(dt); }))];

	auto classDefDescriptor = pragma::LuaCore::register_class<pragma::game::ValueDriverDescriptor>(l, "ValueDriverDescriptor");
	classDefDescriptor->def(luabind::constructor<lua::State *, std::string, std::unordered_map<std::string, std::string>, std::unordered_map<std::string, ::udm::PProperty>>());
	classDefDescriptor->def(luabind::constructor<lua::State *, std::string, std::unordered_map<std::string, std::string>>());
	classDefDescriptor->def(luabind::constructor<lua::State *, std::string>());
	classDefDescriptor->property("expression", static_cast<std::string (*)(lua::State *, pragma::game::ValueDriverDescriptor &)>([](lua::State *l, pragma::game::ValueDriverDescriptor &descriptor) -> std::string { return descriptor.GetExpression(); }),
	  static_cast<void (*)(lua::State *, pragma::game::ValueDriverDescriptor &, const std::string &)>([](lua::State *l, pragma::game::ValueDriverDescriptor &descriptor, const std::string &expr) { descriptor.SetExpression(expr); }));
	classDefDescriptor->def("AddReference", &pragma::game::ValueDriverDescriptor::AddReference);
	classDefDescriptor->def("AddConstant", static_cast<void (*)(pragma::game::ValueDriverDescriptor &, const std::string &, ::udm::PProperty)>([](pragma::game::ValueDriverDescriptor &descriptor, const std::string &name, ::udm::PProperty prop) { descriptor.AddConstant(name, prop); }));
	classDefDescriptor->def("AddConstant", static_cast<void (*)(pragma::game::ValueDriverDescriptor &, const std::string &, const classObject &)>([](pragma::game::ValueDriverDescriptor &descriptor, const std::string &name, const classObject &udmType) {
		for(auto type : ::udm::GENERIC_TYPES) {
			auto r = ::udm::visit<false, true, false>(type, [&udmType, &descriptor, &name](auto tag) mutable -> bool {
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
	classDefDescriptor->def("GetConstants", &pragma::game::ValueDriverDescriptor::GetConstants);
	classDefDescriptor->def("GetReferences", &pragma::game::ValueDriverDescriptor::GetReferences);
	modGame[*classDefDescriptor];
}
