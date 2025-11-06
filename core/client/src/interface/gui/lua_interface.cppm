// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:gui.lua_interface;

export import luabind;
export import pragma.gui;

export class DLLCLIENT WGUILuaInterface {
  private:
	static lua::State *m_guiLuaState;
	static CallbackHandle m_cbGameStart;
	static CallbackHandle m_cbLuaReleased;
	static void OnGUIDestroy(WIBase &el);
	static luabind::object CreateLuaObject(lua::State *l, WIBase &p);
	static void OnGameStart();
	static void OnGameLuaReleased(lua::State *lua);
	static void ClearLuaObjects(WIBase *el);
  public:
	static void Initialize();
	static luabind::object GetLuaObject(lua::State *l, WIBase &p);
	static void InitializeGUIElement(WIBase &p);
	static void Clear();

	static void ClearGUILuaObjects(WIBase &el);
};
