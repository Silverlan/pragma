// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.ents;

import :console.output;
import :engine;
export import :entities.base_entity;
export import :game;
export import :network_state;
export import :physics.shape;
export import :scripting.lua.entity_iterator;

export namespace Lua {
	namespace ents {
		DLLNETWORK LuaEntityIterator create_lua_entity_iterator(lua::State *l, const tb<LuaEntityIteratorFilterBase> &filterTable, pragma::ecs::EntityIterator::FilterFlags filterFlags = pragma::ecs::EntityIterator::FilterFlags::Default);
		DLLNETWORK LuaEntityComponentIterator create_lua_entity_component_iterator(lua::State *l, pragma::ComponentId componentId, const tb<LuaEntityIteratorFilterBase> &filterTable, pragma::ecs::EntityIterator::FilterFlags filterFlags = pragma::ecs::EntityIterator::FilterFlags::Default);

		DLLNETWORK void register_library(lua::State *l);

		DLLNETWORK opt<mult<type<pragma::ecs::BaseEntity>, double>> get_closest(lua::State *l, const Vector3 &origin);
		DLLNETWORK opt<mult<type<pragma::ecs::BaseEntity>, double>> get_farthest(lua::State *l, const Vector3 &origin);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_sorted_by_distance(lua::State *l, const Vector3 &origin);
		DLLNETWORK type<pragma::ecs::BaseEntity> get_random(lua::State *l);
		DLLNETWORK opt<std::string> get_component_name(lua::State *l, pragma::ComponentId componentId);
		DLLNETWORK opt<uint32_t> get_component_id(lua::State *l, const std::string &componentName);

		DLLNETWORK size_t get_lua_component_member_count(pragma::Game &game, pragma::ComponentId componentId);
		DLLNETWORK pragma::ComponentMemberInfo *get_lua_component_member_info(pragma::Game &game, pragma::ComponentId componentId, pragma::ComponentMemberIndex memberIndex);

		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_all(lua::State *l);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_all(lua::State *l, func<type<pragma::ecs::BaseEntity>> func);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_all(lua::State *l, pragma::ecs::EntityIterator::FilterFlags filterFlags = pragma::ecs::EntityIterator::FilterFlags::Default);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_all(lua::State *l, pragma::ecs::EntityIterator::FilterFlags filterFlags, const tb<LuaEntityIteratorFilterBase> &filters);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_all(lua::State *l, const tb<LuaEntityIteratorFilterBase> &filters);
		DLLNETWORK tb<type<pragma::BaseEntityComponent>> get_all_c(lua::State *l, func<type<pragma::BaseEntityComponent>> func);

		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_spawned(lua::State *l);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_players(lua::State *l);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_npcs(lua::State *l);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> get_weapons(lua::State *l);
		DLLNETWORK tb<type<pragma::BaseVehicleComponent>> get_vehicles(lua::State *l);
		DLLNETWORK opt<type<pragma::ecs::BaseEntity>> get_world(lua::State *l);
		DLLNETWORK opt<type<pragma::ecs::BaseEntity>> get_by_index(lua::State *l, uint32_t idx);
		DLLNETWORK opt<type<pragma::ecs::BaseEntity>> get_by_local_index(lua::State *l, uint32_t idx);
		DLLNETWORK opt<type<pragma::ecs::BaseEntity>> find_by_unique_index(lua::State *l, const std::string &uuid);
		DLLNETWORK type<EntityHandle> get_null(lua::State *l);
		DLLNETWORK type<pragma::ecs::BaseEntity> create(lua::State *l, const std::string &classname);
		DLLNETWORK type<pragma::ecs::BaseEntity> create_prop(lua::State *l, const std::string &mdl, const Vector3 *origin, const EulerAngles *angles, bool physicsProp);

		DLLNETWORK type<pragma::ecs::BaseEntity> create_trigger(lua::State *l, const Vector3 &origin, pragma::physics::IShape &shape);
		DLLNETWORK type<pragma::ecs::BaseEntity> create_trigger(lua::State *l, const Vector3 &origin, const Vector3 &min, const Vector3 &max, const EulerAngles &angles);
		DLLNETWORK type<pragma::ecs::BaseEntity> create_trigger(lua::State *l, const Vector3 &origin, float radius);

		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_by_filter(lua::State *l, const std::string &name);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_by_class(lua::State *l, const std::string &className);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_by_name(lua::State *l, const std::string &name);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_by_component(lua::State *l, const std::string &componentName);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_in_aabb(lua::State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_in_sphere(lua::State *l, const Vector3 &origin, float radius);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_in_box(lua::State *l, const Vector3 &min, const Vector3 &max);
		DLLNETWORK tb<type<pragma::ecs::BaseEntity>> find_in_cone(lua::State *l, const Vector3 &origin, const Vector3 &dir, float radius, float angle);
		DLLNETWORK opt<pragma::ComponentEventId> get_event_id(lua::State *l, const std::string &name);
		DLLNETWORK void register_class(lua::State *l, const std::string &className, const classObject &classObject);
		DLLNETWORK void register_class(lua::State *l, const std::string &className, const luabind::tableT<luabind::variant<std::string, pragma::ComponentId>> &tComponents, LuaEntityType type);
		DLLNETWORK opt<pragma::ComponentEventId> register_component_event(lua::State *l, pragma::ComponentId componentId, const std::string &name);
		DLLNETWORK opt<pragma::NetEventId> register_component_net_event(lua::State *l, pragma::ComponentId componentId, const std::string &name);
		template<class TComponent>
		int register_component(lua::State *l)
		{
			std::string name = CheckString(l, 1);
			pragma::string::to_lower(name);
			auto idxClass = 2;
			CheckUserData(l, idxClass);
			auto o = luabind::object(luabind::from_stack(l, idxClass));
			if(!o)
				return 0;
			auto *state = pragma::Engine::Get()->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto &manager = game->GetLuaEntityManager();

			auto &classManager = game->GetLuaClassManager();
			std::string categoryPath;
			auto componentFlags = pragma::ComponentFlags::None;
			if(IsSet(l, 3)) {
				int32_t argFlags = 3;
				if(!IsNumber(l, 3)) {
					categoryPath = CheckString(l, 3);
					++argFlags;
				}
				if(IsSet(l, argFlags))
					componentFlags = static_cast<pragma::ComponentFlags>(CheckInt(l, argFlags));
			}
			componentFlags |= pragma::ComponentFlags::LuaBased;

			// Check if there's an __init-method defined for this class.
			// If there isn't, generate one!
			if(classManager.IsClassMethodDefined(o, "__init") == false) {
				std::string luaStr = "function(self,ent) BaseEntityComponent.__init(self,ent) end";
				std::string err;
				if(PushLuaFunctionFromString(l, luaStr, "ComponentInit", err) == false)
					Con::CWAR << "Unable to register __init method for component class '" << name << "': " << err << Con::endl;
				else {
					CheckFunction(l, -1);
					o["__init"] = luabind::object {luabind::from_stack {l, -1}};

					Pop(l, 1);
				}
			}

			auto firstCreation = true;
			pragma::ComponentRegInfo regInfo {categoryPath};
			auto componentId = game->GetEntityComponentManager().RegisterComponentType(
			  name,
			  [o, game, name, firstCreation](pragma::ecs::BaseEntity &ent) mutable -> pragma::util::TSharedHandle<pragma::BaseEntityComponent> {
				  if(firstCreation) {
					  firstCreation = false;

					  auto &game = *ent.GetNetworkState()->GetGameState();
					  auto &componentManager = game.GetEntityComponentManager();
					  pragma::ComponentId componentId;
					  if(componentManager.GetComponentTypeId(name, componentId)) {
						  auto &componentInfo = *game.GetEntityComponentManager().GetComponentInfo(componentId);
						  if(pragma::math::is_flag_set(componentInfo.flags, pragma::ComponentFlags::Networked | pragma::ComponentFlags::MakeNetworked) == false) {
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
						  if(pragma::math::is_flag_set(componentInfo.flags, pragma::ComponentFlags::MakeNetworked)) {
							  pragma::math::set_flag(componentInfo.flags, pragma::ComponentFlags::MakeNetworked, false);
							  // Note: We could automatically set the component as networked here, but this has several disadvantages, so
							  // we just print a warning instead and require the user to specify the networked-flag when registering the component.
							  // If this behavior should be changed in the future, a 'register_entity_component' net-message will have to be sent
							  // to all clients, in addition to setting the networked-flag below! (See SEntityComponentManager::OnComponentTypeRegistered)
							  // pragma::math::set_flag(componentInfo.flags,pragma::ComponentFlags::Networked);
							  Con::CWAR << "Component '" << name
							            << "' has networked methods or uses net-events, but was not registered as networked, this means networking will be disabled for this component! Set the 'ents.EntityComponent.FREGISTER_BIT_NETWORKED' flag when registering the component to fix this!"
							            << Con::endl;
						  }
					  }
				  }
				  return pragma::util::to_shared_handle<pragma::BaseEntityComponent>(std::shared_ptr<TComponent> {static_cast<TComponent *>(game->CreateLuaEntityComponent(ent, name))});
			  },
			  regInfo, componentFlags);
			manager.RegisterComponent(name, o, componentId);

			luabind::object ents {luabind::globals(l)["ents"]};
			ents["COMPONENT_" + pragma::string::get_upper(name)] = componentId;

			Lua::PushInt(l, componentId);
			return 1;
		}
	};
};
