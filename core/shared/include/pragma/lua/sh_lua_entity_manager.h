// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __SH_LUA_ENTITY_MANAGER_H__
#define __SH_LUA_ENTITY_MANAGER_H__

#include "pragma/networkdefinitions.h"
#include "pragma/types.hpp"
#include <pragma/lua/luaapi.h>
#include <unordered_map>

class DLLNETWORK LuaEntityManager {
  public:
	struct DLLNETWORK EntityInfo {
		luabind::object classObject;
		std::vector<pragma::ComponentId> components;
	};
	void RegisterEntity(std::string className, luabind::object &o, const std::vector<pragma::ComponentId> &components);
	luabind::object *GetClassObject(std::string className);
	EntityInfo *GetEntityInfo(std::string className);

	void RegisterComponent(std::string className, luabind::object &o,pragma::ComponentId componentId);
	luabind::object *GetComponentClassObject(std::string className);
	std::optional<pragma::ComponentId> FindComponentId(const luabind::object &o) const;
	const luabind::object *FindClassObject(pragma::ComponentId componentId) const;

	const std::unordered_map<std::string, EntityInfo> &GetRegisteredEntities() const { return m_ents; }
	const std::unordered_map<std::string, std::pair<luabind::object, pragma::ComponentId>> &GetRegisteredComponents() const { return m_components; }
  private:
	std::unordered_map<std::string, EntityInfo> m_ents;
	std::unordered_map < std::string, std::pair<luabind::object,pragma::ComponentId>> m_components;
};

#endif
