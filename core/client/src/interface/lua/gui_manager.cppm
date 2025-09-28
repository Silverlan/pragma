// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <unordered_map>

export module pragma.client:scripting.lua.gui_manager;
export {
	#pragma warning(push)
	#pragma warning(disable : 4251)
	class DLLCLIENT LuaGUIManager {
	private:
		std::unordered_map<std::string, luabind::object> m_guiElements;
	public:
		void RegisterGUIElement(std::string className, luabind::object &o);
		luabind::object *GetClassObject(std::string className);
	};
	#pragma warning(pop)
};
