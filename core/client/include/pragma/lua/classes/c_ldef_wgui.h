#ifndef __C_LDEF_WGUI_H__
#define __C_LDEF_WGUI_H__

#include "pragma/lua/classes/c_ldef_wguihandles.h"

LUA_SETUP_HANDLE_CHECK(GUIElement,::WIBase,WIHandle);

#define lua_checkgui(l,hGUI) \
	if(!hGUI.IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL GUI Element"); \
		lua_error(l); \
		return; \
	}

DLLCLIENT Con::c_cout & operator<<(Con::c_cout &os,const WIHandle &handle);
DLLCLIENT std::ostream& operator<<(std::ostream &os,const WIHandle &handle);

#endif