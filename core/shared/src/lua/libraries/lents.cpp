/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lents.h"
#include <pragma/engine.h>
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseworld.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/lua/lentity_type.hpp"
#include "pragma/lua/lua_entity_iterator.hpp"
#include "pragma/lua/sh_lua_component.hpp"

extern DLLENGINE Engine *engine;

int Lua::ents::create(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::string classname = luaL_checkstring(l,1);

	BaseEntity *ent = game->CreateEntity(classname);
	if(ent == NULL)
		return 0;
	lua_pushentity(l,ent);
	return 1;
}

int Lua::ents::create_trigger(lua_State *l)
{
	auto origin = *Lua::CheckVector(l,1);
	auto radius = 0.f;
	Vector3 *min = nullptr;
	Vector3 *max = nullptr;
	EulerAngles *ang = nullptr;
	pragma::physics::IShape *lshape = nullptr;
	if(Lua::IsNumber(l,2) == true)
		radius = Lua::CheckNumber(l,2);
	else if(Lua::IsType<pragma::physics::IShape>(l,2))
		lshape = &Lua::Check<pragma::physics::IShape>(l,2);
	else
	{
		min = Lua::CheckVector(l,2);
		max = Lua::CheckVector(l,3);
		ang = Lua::CheckEulerAngles(l,4);
	}
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *phys = game->GetPhysicsEnvironment();
	std::shared_ptr<pragma::physics::IConvexShape> shape = nullptr;
	if(min != nullptr)
	{
		auto extents = (*max) -(*min);
		auto center = ((*max) +(*min)) *0.5f;
		origin += center;
		shape = phys->CreateBoxShape(extents *0.5f,phys->GetGenericMaterial());
	}
	else if(lshape != nullptr)
	{
		if(lshape->IsConvex() == false)
		{
			Con::cwar<<"WARNING: Cannot create trigger_touch entity with non-convex physics shape!"<<Con::endl;
			return 0;
		}
		shape = std::dynamic_pointer_cast<pragma::physics::IConvexShape>(lshape->shared_from_this());
	}
	else
		shape = phys->CreateSphereShape(radius,phys->GetGenericMaterial());
	auto *ent = game->CreateEntity("trigger_touch");
	if(ent == nullptr)
		return 0;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent.valid())
	{
		if(ang != nullptr)
			pTrComponent->SetAngles(*ang);
		pTrComponent->SetPosition(origin);
	}
	ent->SetKeyValue("spawnflags","1144"); // TODO: Use enums
	ent->Spawn();
	if(shape != nullptr)
	{
		auto pPhysComponent = ent->GetPhysicsComponent();
		if(pPhysComponent.valid())
			pPhysComponent->InitializePhysics(*shape);
	}
	lua_pushentity(l,ent);
	return 1;
}

static void push_entities(lua_State *l,std::vector<BaseEntity*> *ents)
{
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	for(int i=0;i<ents->size();i++)
	{
		if((*ents)[i] != NULL)
		{
			lua_pushentity(l,(*ents)[i]);
			lua_rawseti(l,top,n);
			n++;
		}
	}
}

static void push_entities(lua_State *l,EntityIterator &entIt)
{
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto *ent : entIt)
	{
		Lua::PushInt(l,idx++);
		lua_pushentity(l,ent);
		Lua::SetTableValue(l,t);
	}
}

static void iterate_entities(lua_State *l,const std::function<void(BaseEntity*)> &fCallback)
{
	auto fcIterator = 1;
	Lua::CheckFunction(l,fcIterator);
	BaseEntity *ent = nullptr;
	do
	{
		Lua::PushValue(l,fcIterator); /* 1 */
		Lua::Call(l,0,1); /* 1 */
		if(Lua::IsSet(l,-1))
		{
			auto &hEnt = Lua::Check<EntityHandle>(l,-1);
			ent = hEnt.get();
			if(ent != nullptr)
				fCallback(ent);
		}
		else
			ent = nullptr;
		Lua::Pop(l,1); /* 0 */
	}
	while(ent != nullptr);
}

int Lua::ents::get_closest(lua_State *l)
{
	auto &origin = *Lua::CheckVector(l,2);
	auto dClosest = std::numeric_limits<float>::max();
	BaseEntity *entClosest = nullptr;
	iterate_entities(l,[&dClosest,&entClosest,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent.expired())
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		if(d >= dClosest)
			return;
		dClosest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return 0;
	entClosest->GetLuaObject()->push(l);
	Lua::PushNumber(l,dClosest);
	return 2;
}
int Lua::ents::get_farthest(lua_State *l)
{
	auto &origin = *Lua::CheckVector(l,2);
	auto dFarthest = -1.f;
	BaseEntity *entClosest = nullptr;
	iterate_entities(l,[&dFarthest,&entClosest,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent.expired())
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		if(d <= dFarthest)
			return;
		dFarthest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return 0;
	entClosest->GetLuaObject()->push(l);
	Lua::PushNumber(l,dFarthest);
	return 2;
}
int Lua::ents::get_sorted_by_distance(lua_State *l)
{
	auto &origin = *Lua::CheckVector(l,2);
	std::vector<std::pair<BaseEntity*,float>> ents {};
	iterate_entities(l,[&ents,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent.expired())
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		ents.push_back({ent,d});
	});
	std::sort(ents.begin(),ents.end(),[](const std::pair<BaseEntity*,float> &a,const std::pair<BaseEntity*,float> &b) {
		return a.second < b.second;
	});
	auto t = Lua::CreateTable(l);
	auto idx = 1;
	for(auto &pair : ents)
	{
		Lua::PushInt(l,idx++);
		pair.first->GetLuaObject()->push(l);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::ents::get_random(lua_State *l)
{
	std::vector<BaseEntity*> ents {};
	iterate_entities(l,[&ents](BaseEntity *ent) {ents.push_back(ent);});
	if(ents.empty())
		return 0;
	auto r = umath::random(0,ents.size() -1);
	auto *ent = ents.at(r);
	ent->GetLuaObject()->push(l);
	return 1;
}

int Lua::ents::get_component_name(lua_State *l)
{
	auto componentId = Lua::CheckInt(l,1);
	auto *info = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentInfo(componentId);
	if(info == nullptr)
		return 0;
	Lua::PushString(l,info->name);
	return 1;
}
int Lua::ents::get_component_id(lua_State *l)
{
	auto *componentName = Lua::CheckString(l,1);
	pragma::ComponentId componentId;
	if(engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentTypeId(componentName,componentId) == false)
		return 0;
	Lua::PushInt(l,componentId);
	return 1;
}

int Lua::ents::get_all(lua_State *l)
{
	if(Lua::IsSet(l,1))
	{
		if(Lua::IsFunction(l,1))
		{
			std::vector<BaseEntity*> ents {};
			iterate_entities(l,[&ents](BaseEntity *ent) {ents.push_back(ent);});
			if(ents.empty())
				return 0;
			auto t = Lua::CreateTable(l);
			auto idx = 1;
			for(auto *ent : ents)
			{
				Lua::PushInt(l,idx++);
				ent->GetLuaObject()->push(l);
				Lua::SetTableValue(l,t);
			}
			return 1;
		}
		if(Lua::IsSet(l,2) == false)
		{
			auto filterFlags = EntityIterator::FilterFlags::Default;
			auto filterIdx = 1u;
			if(Lua::IsNumber(l,1))
			{
				filterFlags = static_cast<EntityIterator::FilterFlags>(Lua::CheckInt(l,1));
				filterIdx = std::numeric_limits<uint32_t>::max();
			}
			auto it = create_lua_entity_iterator(l,luabind::object(luabind::from_stack(l,1)),filterIdx,filterFlags);

			auto t = Lua::CreateTable(l);
			auto idx = 1u;
			for(auto o : it)
			{
				Lua::PushInt(l,idx++);
				o.push(l);
				Lua::SetTableValue(l,t);
			}
			return 1;
		}
		Lua::CheckInt(l,1);
		auto filterFlags = static_cast<EntityIterator::FilterFlags>(Lua::CheckInt(l,1));
		auto it = create_lua_entity_iterator(l,luabind::object(luabind::from_stack(l,2)),2u,filterFlags);
		auto t = Lua::CreateTable(l);
		auto idx = 1u;
		for(auto o : it)
		{
			Lua::PushInt(l,idx++);
			o.push(l);
			Lua::SetTableValue(l,t);
		}
		return 1;
	}
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<BaseEntity*> *ents;
	game->GetEntities(&ents);
	push_entities(l,ents);
	return 1;
}

int Lua::ents::get_spawned(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<BaseEntity*> ents;
	game->GetSpawnedEntities(&ents);
	push_entities(l,&ents);
	return 1;
}

int Lua::ents::get_players(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Player};
	for(auto *ent : it)
	{
		Lua::PushInt(l,idx++);
		ent->GetPlayerComponent().get()->PushLuaObject(l);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::ents::get_npcs(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::NPC};
	for(auto *ent : it)
	{
		Lua::PushInt(l,idx++);
		ent->GetAIComponent().get()->PushLuaObject(l);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::ents::get_weapons(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Weapon};
	for(auto *ent : it)
	{
		Lua::PushInt(l,idx++);
		ent->GetWeaponComponent().get()->PushLuaObject(l);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::ents::get_vehicles(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Vehicle};
	for(auto *ent : it)
	{
		Lua::PushInt(l,idx++);
		ent->GetVehicleComponent().get()->PushLuaObject(l);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::ents::get_world(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	auto *pWorld = game->GetWorld();
	if(pWorld == nullptr)
		return 0;
	BaseEntity &world = pWorld->GetEntity();
	lua_pushentity(l,(&world));
	return 1;
}

int Lua::ents::get_by_index(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	int idx = Lua::CheckInt<int>(l,1);
	BaseEntity *ent = game->GetEntity(idx);
	if(ent == NULL)
		return 0;
	lua_pushentity(l,ent);
	return 1;
}

int Lua::ents::get_by_local_index(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	int idx = Lua::CheckInt<int>(l,1);
	BaseEntity *ent = game->GetEntityByLocalIndex(idx);
	if(ent == NULL)
		return 0;
	lua_pushentity(l,ent);
	return 1;
}

int Lua::ents::find_by_unique_index(lua_State *l)
{
	auto uniqueIndex = Lua::CheckInt(l,1);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	EntityIterator entIt {*game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	auto it = std::find_if(entIt.begin(),entIt.end(),[uniqueIndex](const BaseEntity *ent) {
		return ent->GetUniqueIndex() == uniqueIndex;
	});
	if(it == entIt.end())
		return 0;
	lua_pushentity(l,(*it));
	return 1;
}

int Lua::ents::get_null(lua_State *l)
{
	Lua::Push<EntityHandle>(l,EntityHandle{});
	return 1;
}

int Lua::ents::find_by_filter(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterEntity>(name);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_by_class(lua_State *l)
{
	std::string classname = luaL_checkstring(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterClass>(classname);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_by_name(lua_State *l)
{
	std::string name = luaL_checkstring(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterName>(name);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_by_component(lua_State *l)
{
	std::string componentName = Lua::CheckString(l,1);
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();

	auto t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto *ent : EntityIterator{*game,componentName})
	{
		Lua::PushInt(l,idx++);
		lua_pushentity(l,ent);
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::ents::find_in_aabb(lua_State *l)
{
	auto &min = *Lua::CheckVector(l,1);
	auto &max = *Lua::CheckVector(l,2);

	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterBox>(min,max);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_in_sphere(lua_State *l)
{
	Vector3 *origin = Lua::CheckVector(l,1);
	float radius = Lua::CheckNumber<float>(l,2);
	std::vector<BaseEntity*> ents;
	NetworkState *state = engine->GetNetworkState(l);

	EntityIterator entIt {*state->GetGameState()};
	entIt.AttachFilter<EntityIteratorFilterSphere>(*origin,radius);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_in_box(lua_State *l)
{
	Vector3 *min = Lua::CheckVector(l,1);
	Vector3 *max = Lua::CheckVector(l,2);
	std::vector<BaseEntity*> ents;
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterBox>(*min,*max);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::find_in_cone(lua_State *l)
{
	auto *origin = Lua::CheckVector(l,1);
	auto *dir = Lua::CheckVector(l,2);
	auto radius = Lua::CheckNumber(l,3);
	auto angle = Lua::CheckNumber(l,4);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterCone>(*origin,*dir,radius,angle);
	push_entities(l,entIt);
	return 1;
}

int Lua::ents::register_event(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	auto eventId = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().RegisterEvent(name);
	Lua::PushInt(l,eventId);
	return 1;
}
int Lua::ents::get_event_id(lua_State *l)
{
	std::string name = Lua::CheckString(l,1);
	pragma::ComponentEventId eventId;
	if(engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetEventId(name,eventId) == false)
		return 0;
	Lua::PushInt(l,eventId);
	return 1;
}

int Lua::ents::register_class(lua_State *l)
{
	auto className = Lua::CheckString(l,1);
	if(Lua::IsUserData(l,2))
	{
		// Legacy system
		std::string className = Lua::CheckString(l,1);
		Lua::CheckUserData(l,2);
		auto o = luabind::object(luabind::from_stack(l,2));
		if(o)
		{
			auto *state = engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto &manager = game->GetLuaEntityManager();
			manager.RegisterEntity(className,o);
		}
		return 0;
	}

	auto t = 2;
	Lua::CheckTable(l,t);
	std::vector<pragma::ComponentId> components;
	auto numComponents = Lua::GetObjectLength(l,t);
	components.reserve(numComponents);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	for(auto i=decltype(numComponents){0};i<numComponents;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		if(Lua::IsNumber(l,-1))
		{
			auto id = Lua::CheckInt(l,-1);
			if(id != pragma::INVALID_COMPONENT_ID)
				components.push_back(id);
			else
				Con::cwar<<"WARNING: Attempted to add unknown entity component to registration of entity '"<<className<<"'! Skipping..."<<Con::endl;
		}
		else
		{
			std::string name = Lua::CheckString(l,-1);
			auto id = pragma::INVALID_COMPONENT_ID;
			if(componentManager.GetComponentTypeId(name,id,false))
				components.push_back(id);
			else
			{
				if(game->LoadLuaComponentByName(name) == true && componentManager.GetComponentTypeId(name,id,false) == true)
					components.push_back(id);
				else
					Con::cwar<<"WARNING: Attempted to add unknown entity component '"<<name<<"' to registration of entity '"<<className<<"'! Skipping..."<<Con::endl;
			}
		}
		Lua::Pop(l,1);
	}
	const char *cLuaClassName = "__TMP_ENTITY_CLASS";
	std::string luaClassName = cLuaClassName;
	std::stringstream ss;
	ss<<"util.register_class(\""<<luaClassName<<"\",BaseEntity);\n";
	ss<<"function "<<luaClassName<<":__init()\n";
	ss<<"    BaseEntity.__init(self)\n";
	ss<<"end\n";
	ss<<"function "<<luaClassName<<":Initialize()\n";
	ss<<"    BaseEntity.Initialize(self)\n";
	for(auto componentId : components)
		ss<<"    self:AddComponent("<<componentId<<")\n";
	ss<<"end\n";

	auto r = Lua::RunString(l,ss.str(),"internal");
	if(r == Lua::StatusCode::Ok)
	{
		auto o = luabind::object(luabind::globals(l)[cLuaClassName]);
		if(o)
		{
			auto type = LuaEntityType::Default;
			if(Lua::IsSet(l,3))
				type = static_cast<LuaEntityType>(Lua::CheckInt(l,3));
			o["Type"] = umath::to_integral(type);

			auto &manager = game->GetLuaEntityManager();
			manager.RegisterEntity(className,o);
		}
	}
	else
		Lua::HandleLuaError(l);

	Lua::PushNil(l);
	Lua::SetGlobal(l,luaClassName);
	return 0;
}

int Lua::ents::register_component_event(lua_State *l)
{
	auto componentId = Lua::CheckInt(l,1);
	auto *name = Lua::CheckString(l,2);

	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto *componentInfo = componentManager.GetComponentInfo(componentId);
	if(componentInfo == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to register component event '"<<name<<"' to unknown component type "<<componentId<<"!"<<Con::endl;
		return 0;
	}

	auto netName = componentInfo->name +'_' +std::string{name};
	auto eventId = componentManager.RegisterEvent(netName);
	Lua::PushInt(l,eventId);
	return 1;
}
