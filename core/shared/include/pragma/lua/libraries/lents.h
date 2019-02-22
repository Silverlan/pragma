#ifndef __LENTS_H__
#define __LENTS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/entities/entity_iterator.hpp"

extern DLLENGINE Engine *engine;

class LuaEntityIterator;
namespace Lua
{
	namespace ents
	{
		DLLNETWORK LuaEntityIterator create_lua_entity_iterator(lua_State *l,luabind::object oFilter,uint32_t idxFilter,EntityIterator::FilterFlags filterFlags=EntityIterator::FilterFlags::Default);
		
		DLLNETWORK int get_closest(lua_State *l);
		DLLNETWORK int get_farthest(lua_State *l);
		DLLNETWORK int get_sorted_by_distance(lua_State *l);
		DLLNETWORK int get_random(lua_State *l);

		DLLNETWORK int get_all(lua_State *l);
		DLLNETWORK int get_spawned(lua_State *l);
		DLLNETWORK int get_players(lua_State *l);
		DLLNETWORK int get_npcs(lua_State *l);
		DLLNETWORK int get_weapons(lua_State *l);
		DLLNETWORK int get_vehicles(lua_State *l);
		DLLNETWORK int get_world(lua_State *l);
		DLLNETWORK int get_by_index(lua_State *l);
		DLLNETWORK int find_by_unique_index(lua_State *l);
		DLLNETWORK int get_null(lua_State *l);
		DLLNETWORK int create(lua_State *l);
		DLLNETWORK int create_trigger(lua_State *l);
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
		template<class TComponent>
			int register_component(lua_State *l)
		{
			std::string name = Lua::CheckString(l,1);
			ustring::to_lower(name);
			Lua::CheckUserData(l,2);
			auto o = luabind::object(luabind::from_stack(l,2));
			if(!o)
				return 0;
			auto *state = ::engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto &manager = game->GetLuaEntityManager();
			manager.RegisterComponent(name,o);

			auto componentFlags = pragma::ComponentFlags::None;
			if(Lua::IsSet(l,3))
				componentFlags = static_cast<pragma::ComponentFlags>(Lua::CheckInt(l,3));

			auto componentId = game->GetEntityComponentManager().RegisterComponentType(name,[o,game,name](BaseEntity &ent) -> std::shared_ptr<pragma::BaseEntityComponent> {
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
	{"find_by_unique_index",Lua::ents::find_by_unique_index}, \
	{"get_null",Lua::ents::get_null}, \
	{"find_by_filter",Lua::ents::find_by_filter}, \
	{"find_by_class",Lua::ents::find_by_class}, \
	{"find_by_name",Lua::ents::find_by_name}, \
	{"find_by_component",Lua::ents::find_by_component}, \
	{"find_in_sphere",Lua::ents::find_in_sphere}, \
	{"find_in_aabb",Lua::ents::find_in_aabb}, \
	{"find_in_cone",Lua::ents::find_in_cone}, \
	{"create",Lua::ents::create}, \
	{"create_trigger",Lua::ents::create_trigger}, \
	{"register",Lua::ents::register_class}, \
	{"get_closest",Lua::ents::get_closest}, \
	{"get_farthest",Lua::ents::get_farthest}, \
	{"get_sorted_by_distance",Lua::ents::get_sorted_by_distance}, \
	{"get_random",Lua::ents::get_random},

#endif
