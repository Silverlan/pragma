#ifndef __WGUI_LUAINTERFACE_H__
#define __WGUI_LUAINTERFACE_H__

#include "pragma/clientdefinitions.h"
#include "luasystem.h"

class WIBase;
class CGame;
class DLLCLIENT WGUILuaInterface
{
private:
	static lua_State *m_guiLuaState;
	static CallbackHandle m_cbGameStart;
	static CallbackHandle m_cbLuaReleased;
	static void OnGUIDestroy(WIBase &el);
	static luabind::object CreateLuaObject(lua_State *l,WIBase &p);
	static void OnGameStart();
	static void OnGameLuaReleased(lua_State *lua);
	static void ClearLuaObjects(WIBase *el);
public:
	static void Initialize();
	static luabind::object GetLuaObject(lua_State *l,WIBase &p);
	static void InitializeGUIElement(WIBase &p);
	static void Clear();

	static void ClearGUILuaObjects(WIBase &el);
};

#endif