/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __LENTS_H__
#define __LENTS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/lua/class_manager.hpp"

extern DLLNETWORK Engine *engine;

class LuaEntityIterator;
using LuaEntityObject = luabind::object;
namespace Lua
{
	namespace ents
	{
		DLLNETWORK LuaEntityIterator create_lua_entity_iterator(lua_State *l,luabind::object oFilter,uint32_t idxFilter,EntityIterator::FilterFlags filterFlags=EntityIterator::FilterFlags::Default);
		
		DLLNETWORK void register_library(lua_State *l);

		DLLNETWORK int get_closest(lua_State *l);
		DLLNETWORK int get_farthest(lua_State *l);
		DLLNETWORK int get_sorted_by_distance(lua_State *l);
		DLLNETWORK int get_random(lua_State *l);
		DLLNETWORK int get_component_name(lua_State *l);
		DLLNETWORK int get_component_id(lua_State *l);

		DLLNETWORK int get_all(lua_State *l);
		DLLNETWORK int get_spawned(lua_State *l);
		DLLNETWORK int get_players(lua_State *l);
		DLLNETWORK int get_npcs(lua_State *l);
		DLLNETWORK int get_weapons(lua_State *l);
		DLLNETWORK int get_vehicles(lua_State *l);
		DLLNETWORK int get_world(lua_State *l);
		DLLNETWORK int get_by_index(lua_State *l);
		DLLNETWORK int get_by_local_index(lua_State *l);
		DLLNETWORK int find_by_unique_index(lua_State *l);
		DLLNETWORK int get_null(lua_State *l);
		DLLNETWORK LuaEntityObject create(lua_State *l,const std::string &classname);
		DLLNETWORK int create_trigger(lua_State *l);
		DLLNETWORK int create_prop(lua_State *l);
		DLLNETWORK int find_by_filter(lua_State *l);
		DLLNETWORK int find_by_class(lua_State *l);
		DLLNETWORK int find_by_name(lua_State *l);
		DLLNETWORK int find_by_component(lua_State *l);
		DLLNETWORK int find_in_aabb(lua_State *l);
		DLLNETWORK int find_in_sphere(lua_State *l);
		DLLNETWORK int find_in_box(lua_State *l);
		DLLNETWORK int find_in_cone(lua_State *l);
		DLLNETWORK int register_event(lua_State *l);
		DLLNETWORK int get_event_id(lua_State *l);
		DLLNETWORK int register_class(lua_State *l);
		DLLNETWORK int register_component_event(lua_State *l);
		DLLNETWORK int register_component_net_event(lua_State *l);
		template<class TComponent>
			int register_component(lua_State *l)
		{
			std::string name = Lua::CheckString(l,1);
			ustring::to_lower(name);
			auto idxClass = 2;
			Lua::CheckUserData(l,idxClass);
			auto o = luabind::object(luabind::from_stack(l,idxClass));
			if(!o)
				return 0;
			auto *state = ::engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto &manager = game->GetLuaEntityManager();
			manager.RegisterComponent(name,o);

			auto &classManager = game->GetLuaClassManager();
			auto componentFlags = pragma::ComponentFlags::None;
			if(Lua::IsSet(l,3))
				componentFlags = static_cast<pragma::ComponentFlags>(Lua::CheckInt(l,3));

			// Check if there's an __init-method defined for this class.
			// If there isn't, generate one!
			if(classManager.IsClassMethodDefined(o,"__init") == false)
			{
				std::string luaStr = "function(self) BaseEntityComponent.__init(self) end";
				std::string err;
				if(Lua::PushLuaFunctionFromString(l,luaStr,"ComponentInit",err) == false)
					Con::cwar<<"WARNING: Unable to register __init method for component class '"<<name<<"': "<<err<<Con::endl;
				else
				{
					Lua::CheckFunction(l,-1);
					o["__init"] = luabind::object{luabind::from_stack{l,-1}};

					Lua::Pop(l,1);
				}
			}

			auto firstCreation = true;
			auto componentId = game->GetEntityComponentManager().RegisterComponentType(name,[o,game,name,firstCreation](BaseEntity &ent) mutable -> std::shared_ptr<pragma::BaseEntityComponent> {
				if(firstCreation)
				{
					firstCreation = false;

					auto &game = *ent.GetNetworkState()->GetGameState();
					auto &componentManager = game.GetEntityComponentManager();
					pragma::ComponentId componentId;
					if(componentManager.GetComponentTypeId(name,componentId))
					{
						auto &componentInfo = *game.GetEntityComponentManager().GetComponentInfo(componentId);
						if(umath::is_flag_set(componentInfo.flags,pragma::ComponentFlags::Networked | pragma::ComponentFlags::MakeNetworked) == false)
						{
							// Component has not been marked as networked, check if it has any networked
							// methods to be sure.
							std::vector<std::string> networkedMethodNames {};
							if(game.IsClient())
							{
								networkedMethodNames.reserve(3);
								networkedMethodNames.push_back("ReceiveData");
								networkedMethodNames.push_back("ReceiveNetEvent");
								networkedMethodNames.push_back("ReceiveSnapshotData");
							}
							else
							{
								networkedMethodNames.reserve(3);
								networkedMethodNames.push_back("SendData");
								networkedMethodNames.push_back("ReceiveNetEvent");
								networkedMethodNames.push_back("SendSnapshotData");
							}
							auto &classManager = game.GetLuaClassManager();
							auto it = std::find_if(networkedMethodNames.begin(),networkedMethodNames.end(),[&classManager,&o](const std::string &methodName) {
								return classManager.IsClassMethodDefined(o,methodName);
								});
							if(it != networkedMethodNames.end())
								componentInfo.flags |= pragma::ComponentFlags::MakeNetworked;
						}
						if(umath::is_flag_set(componentInfo.flags,pragma::ComponentFlags::MakeNetworked))
						{
							umath::set_flag(componentInfo.flags,pragma::ComponentFlags::MakeNetworked,false);
							// Note: We could automatically set the component as networked here, but this has several disadvantages, so
							// we just print a warning instead and require the user to specify the networked-flag when registering the component.
							// If this behavior should be changed in the future, a 'register_entity_component' net-message will have to be sent
							// to all clients, in addition to setting the networked-flag below! (See SEntityComponentManager::OnComponentTypeRegistered)
							// umath::set_flag(componentInfo.flags,pragma::ComponentFlags::Networked);
							Con::cwar<<"WARNING: Component '"<<name<<"' has networked methods or uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"<<Con::endl;
						}
					}
				}
				return std::static_pointer_cast<pragma::BaseEntityComponent>(std::shared_ptr<TComponent>(static_cast<TComponent*>(game->CreateLuaEntityComponent(ent,name))));
			},componentFlags);
			Lua::PushInt(l,componentId);
			return 1;
		}
	};
};

#define LUA_LIB_ENTS_SHARED \
	{"get_all",Lua::ents::get_all}, \
	{"get_spawned",Lua::ents::get_spawned}, \
	{"get_players",Lua::ents::get_players}, \
	{"get_npcs",Lua::ents::get_npcs}, \
	{"get_weapons",Lua::ents::get_weapons}, \
	{"get_vehicles",Lua::ents::get_vehicles}, \
	{"get_world",Lua::ents::get_world}, \
	{"get_by_index",Lua::ents::get_by_index}, \
	{"get_by_local_index",Lua::ents::get_by_local_index}, \
	{"find_by_unique_index",Lua::ents::find_by_unique_index}, \
	{"get_null",Lua::ents::get_null}, \
	{"find_by_filter",Lua::ents::find_by_filter}, \
	{"find_by_class",Lua::ents::find_by_class}, \
	{"find_by_name",Lua::ents::find_by_name}, \
	{"find_by_component",Lua::ents::find_by_component}, \
	{"find_in_sphere",Lua::ents::find_in_sphere}, \
	{"find_in_aabb",Lua::ents::find_in_aabb}, \
	{"find_in_cone",Lua::ents::find_in_cone}, \
	{"create_trigger",Lua::ents::create_trigger}, \
	{"create_prop",Lua::ents::create_prop}, \
	{"register",Lua::ents::register_class}, \
	{"get_closest",Lua::ents::get_closest}, \
	{"get_farthest",Lua::ents::get_farthest}, \
	{"get_sorted_by_distance",Lua::ents::get_sorted_by_distance}, \
	{"get_random",Lua::ents::get_random}, \
	{"get_component_name",Lua::ents::get_component_name}, \
	{"get_component_id",Lua::ents::get_component_id}, \
	{"register_component_net_event",Lua::ents::register_component_net_event},

#endif
