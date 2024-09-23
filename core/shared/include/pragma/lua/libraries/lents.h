/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LENTS_H__
#define __LENTS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/lua/class_manager.hpp"

extern DLLNETWORK Engine *engine;

class LuaEntityIterator;
class LuaEntityComponentIterator;
using LuaEntityObject = luabind::object;
struct LuaEntityIteratorFilterBase;
enum class LuaEntityType : uint8_t;
namespace Lua {
	namespace ents {
		DLLNETWORK LuaEntityIterator create_lua_entity_iterator(lua_State *l, const tb<LuaEntityIteratorFilterBase> &filterTable, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		DLLNETWORK LuaEntityComponentIterator create_lua_entity_component_iterator(lua_State *l, pragma::ComponentId componentId, const tb<LuaEntityIteratorFilterBase> &filterTable, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);

		DLLNETWORK void register_library(lua_State *l);

		DLLNETWORK opt<mult<type<BaseEntity>, double>> get_closest(lua_State *l, const Vector3 &origin);
		DLLNETWORK opt<mult<type<BaseEntity>, double>> get_farthest(lua_State *l, const Vector3 &origin);
		DLLNETWORK tb<type<BaseEntity>> get_sorted_by_distance(lua_State *l, const Vector3 &origin);
		DLLNETWORK type<BaseEntity> get_random(lua_State *l);
		DLLNETWORK opt<std::string> get_component_name(lua_State *l, pragma::ComponentId componentId);
		DLLNETWORK opt<uint32_t> get_component_id(lua_State *l, const std::string &componentName);

		DLLNETWORK size_t get_lua_component_member_count(Game &game, pragma::ComponentId componentId);
		DLLNETWORK pragma::ComponentMemberInfo *get_lua_component_member_info(Game &game, pragma::ComponentId componentId, pragma::ComponentMemberIndex memberIndex);

		DLLNETWORK tb<type<BaseEntity>> get_all(lua_State *l);
		DLLNETWORK tb<type<BaseEntity>> get_all(lua_State *l, func<type<BaseEntity>> func);
		DLLNETWORK tb<type<BaseEntity>> get_all(lua_State *l, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		DLLNETWORK tb<type<BaseEntity>> get_all(lua_State *l, EntityIterator::FilterFlags filterFlags, const tb<LuaEntityIteratorFilterBase> &filters);
		DLLNETWORK tb<type<BaseEntity>> get_all(lua_State *l, const tb<LuaEntityIteratorFilterBase> &filters);
		DLLNETWORK tb<type<pragma::BaseEntityComponent>> get_all_c(lua_State *l, func<type<pragma::BaseEntityComponent>> func);

		DLLNETWORK tb<type<BaseEntity>> get_spawned(lua_State *l);
		DLLNETWORK tb<type<BaseEntity>> get_players(lua_State *l);
		DLLNETWORK tb<type<BaseEntity>> get_npcs(lua_State *l);
		DLLNETWORK tb<type<BaseEntity>> get_weapons(lua_State *l);
		DLLNETWORK tb<type<pragma::BaseVehicleComponent>> get_vehicles(lua_State *l);
		DLLNETWORK opt<type<BaseEntity>> get_world(lua_State *l);
		DLLNETWORK opt<type<BaseEntity>> get_by_index(lua_State *l, uint32_t idx);
		DLLNETWORK opt<type<BaseEntity>> get_by_local_index(lua_State *l, uint32_t idx);
		DLLNETWORK opt<type<BaseEntity>> find_by_unique_index(lua_State *l, const std::string &uuid);
		DLLNETWORK type<EntityHandle> get_null(lua_State *l);
		DLLNETWORK type<BaseEntity> create(lua_State *l, const std::string &classname);
		DLLNETWORK Lua::type<BaseEntity> create_prop(lua_State *l, const std::string &mdl, const Vector3 *origin, const EulerAngles *angles, bool physicsProp);

		DLLNETWORK type<BaseEntity> create_trigger(lua_State *l, const Vector3 &origin, pragma::physics::IShape &shape);
		DLLNETWORK type<BaseEntity> create_trigger(lua_State *l, const Vector3 &origin, const Vector3 &min, const Vector3 &max, const EulerAngles &angles);
		DLLNETWORK type<BaseEntity> create_trigger(lua_State *l, const Vector3 &origin, float radius);

		DLLNETWORK tb<type<BaseEntity>> find_by_filter(lua_State *l, const std::string &name);
		DLLNETWORK tb<type<BaseEntity>> find_by_class(lua_State *l, const std::string &className);
		DLLNETWORK tb<type<BaseEntity>> find_by_name(lua_State *l, const std::string &name);
		DLLNETWORK tb<type<BaseEntity>> find_by_component(lua_State *l, const std::string &componentName);
		DLLNETWORK tb<type<BaseEntity>> find_in_aabb(lua_State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK tb<type<BaseEntity>> find_in_sphere(lua_State *l, const Vector3 &origin, float radius);
		DLLNETWORK tb<type<BaseEntity>> find_in_box(lua_State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK tb<type<BaseEntity>> find_in_cone(lua_State *l, const Vector3 &origin, const Vector3 &dir, float radius, float angle);
		DLLNETWORK opt<pragma::ComponentEventId> get_event_id(lua_State *l, const std::string &name);
		DLLNETWORK void register_class(lua_State *l, const std::string &className, const Lua::classObject &classObject);
		DLLNETWORK void register_class(lua_State *l, const std::string &className, const luabind::tableT<luabind::variant<std::string, pragma::ComponentId>> &tComponents, LuaEntityType type);
		DLLNETWORK opt<pragma::ComponentEventId> register_component_event(lua_State *l, pragma::ComponentId componentId, const std::string &name);
		DLLNETWORK opt<pragma::NetEventId> register_component_net_event(lua_State *l, pragma::ComponentId componentId, const std::string &name);
		template<class TComponent>
		int register_component(lua_State *l)
		{
			std::string name = Lua::CheckString(l, 1);
			ustring::to_lower(name);
			auto idxClass = 2;
			Lua::CheckUserData(l, idxClass);
			auto o = luabind::object(luabind::from_stack(l, idxClass));
			if(!o)
				return 0;
			auto *state = ::engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto &manager = game->GetLuaEntityManager();

			auto &classManager = game->GetLuaClassManager();
			std::string categoryPath;
			auto componentFlags = pragma::ComponentFlags::None;
			if(Lua::IsSet(l, 3)) {
				int32_t argFlags = 3;
				if(!Lua::IsNumber(l, 3)) {
					categoryPath = Lua::CheckString(l, 3);
					++argFlags;
				}
				if(Lua::IsSet(l, argFlags))
					componentFlags = static_cast<pragma::ComponentFlags>(Lua::CheckInt(l, argFlags));
			}
			componentFlags |= pragma::ComponentFlags::LuaBased;

			// Check if there's an __init-method defined for this class.
			// If there isn't, generate one!
			if(classManager.IsClassMethodDefined(o, "__init") == false) {
				std::string luaStr = "function(self,ent) BaseEntityComponent.__init(self,ent) end";
				std::string err;
				if(Lua::PushLuaFunctionFromString(l, luaStr, "ComponentInit", err) == false)
					Con::cwar << "Unable to register __init method for component class '" << name << "': " << err << Con::endl;
				else {
					Lua::CheckFunction(l, -1);
					o["__init"] = luabind::object {luabind::from_stack {l, -1}};

					Lua::Pop(l, 1);
				}
			}

			auto firstCreation = true;
			pragma::ComponentRegInfo regInfo {categoryPath};
			auto componentId = game->GetEntityComponentManager().RegisterComponentType(
			  name,
			  [o, game, name, firstCreation](BaseEntity &ent) mutable -> util::TSharedHandle<pragma::BaseEntityComponent> {
				  if(firstCreation) {
					  firstCreation = false;

					  auto &game = *ent.GetNetworkState()->GetGameState();
					  auto &componentManager = game.GetEntityComponentManager();
					  pragma::ComponentId componentId;
					  if(componentManager.GetComponentTypeId(name, componentId)) {
						  auto &componentInfo = *game.GetEntityComponentManager().GetComponentInfo(componentId);
						  if(umath::is_flag_set(componentInfo.flags, pragma::ComponentFlags::Networked | pragma::ComponentFlags::MakeNetworked) == false) {
							  // Component has not been marked as networked, check if it has any networked
							  // methods to be sure.
							  std::vector<std::string> networkedMethodNames {};
							  if(game.IsClient()) {
								  networkedMethodNames.reserve(3);
								  networkedMethodNames.push_back("ReceiveData");
								  networkedMethodNames.push_back("ReceiveNetEvent");
								  networkedMethodNames.push_back("ReceiveSnapshotData");
							  }
							  else {
								  networkedMethodNames.reserve(3);
								  networkedMethodNames.push_back("SendData");
								  networkedMethodNames.push_back("ReceiveNetEvent");
								  networkedMethodNames.push_back("SendSnapshotData");
							  }
							  auto &classManager = game.GetLuaClassManager();
							  auto it = std::find_if(networkedMethodNames.begin(), networkedMethodNames.end(), [&classManager, &o](const std::string &methodName) { return classManager.IsClassMethodDefined(o, methodName); });
							  if(it != networkedMethodNames.end())
								  componentInfo.flags |= pragma::ComponentFlags::MakeNetworked;
						  }
						  if(umath::is_flag_set(componentInfo.flags, pragma::ComponentFlags::MakeNetworked)) {
							  umath::set_flag(componentInfo.flags, pragma::ComponentFlags::MakeNetworked, false);
							  // Note: We could automatically set the component as networked here, but this has several disadvantages, so
							  // we just print a warning instead and require the user to specify the networked-flag when registering the component.
							  // If this behavior should be changed in the future, a 'register_entity_component' net-message will have to be sent
							  // to all clients, in addition to setting the networked-flag below! (See SEntityComponentManager::OnComponentTypeRegistered)
							  // umath::set_flag(componentInfo.flags,pragma::ComponentFlags::Networked);
							  Con::cwar << "Component '" << name
							            << "' has networked methods or uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"
							            << Con::endl;
						  }
					  }
				  }
				  return util::to_shared_handle<pragma::BaseEntityComponent>(std::shared_ptr<TComponent> {static_cast<TComponent *>(game->CreateLuaEntityComponent(ent, name))});
			  },
			  regInfo, componentFlags);
			manager.RegisterComponent(name, o, componentId);

			luabind::object ents {luabind::globals(l)["ents"]};
			ents["COMPONENT_" + ustring::get_upper(name)] = componentId;

			Lua::PushInt(l, componentId);
			return 1;
		}
	};
};

#endif
