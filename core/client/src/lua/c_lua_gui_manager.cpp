#include "stdafx_client.h"
#include "pragma/lua/c_lua_gui_manager.h"
#include "pragma/lua/libraries/c_lgui.h"

void LuaGUIManager::RegisterGUIElement(std::string className,luabind::object &o)
{
	ustring::to_lower(className);
	m_guiElements[className] = o;
	//m_guiElements.insert(std::unordered_map<std::string,luabind::object>::value_type(className,o));
}

luabind::object *LuaGUIManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_guiElements.find(className);
	if(it == m_guiElements.end())
		return nullptr;
	return &it->second;
}
