// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.lua_interface;

export import pragma.lua;
export import pragma.gui;

export namespace pragma::gui {
	class DLLCLIENT WGUILuaInterface {
	  private:
		static lua::State *m_guiLuaState;
		static CallbackHandle m_cbGameStart;
		static CallbackHandle m_cbLuaReleased;
		static void OnGUIDestroy(Element &el);
		static luabind::object CreateLuaObject(lua::State *l, Element &p);
		static void OnGameStart();
		static void OnGameLuaReleased(lua::State *lua);
		static void ClearLuaObjects(Element *el);
	  public:
		static void Initialize();
		static luabind::object GetLuaObject(lua::State *l, Element &p);
		static void InitializeGUIElement(Element &p);
		static void Clear();

		static void ClearGUILuaObjects(Element &el);
	};
}
