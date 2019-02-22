#ifndef __SH_LUA_ENTITY_MANAGER_H__
#define __SH_LUA_ENTITY_MANAGER_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <unordered_map>

class DLLNETWORK LuaEntityManager
{
private:
	std::unordered_map<std::string,luabind::object> m_ents;
	std::unordered_map<std::string,luabind::object> m_components;
public:
	void RegisterEntity(std::string className,luabind::object &o);
	luabind::object *GetClassObject(std::string className);

	void RegisterComponent(std::string className,luabind::object &o);
	luabind::object *GetComponentClassObject(std::string className);
};

#endif