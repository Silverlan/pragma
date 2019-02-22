#ifndef _C_LGUI_H__
#define _C_LGUI_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace gui
	{
		DLLCLIENT int create(lua_State *l);
		DLLCLIENT int create_label(lua_State *l);
		DLLCLIENT int register_element(lua_State *l);
		DLLCLIENT int get_base_element(lua_State *l);
		DLLCLIENT int get_focused_element(lua_State *l);
		DLLCLIENT int register_skin(lua_State *l);
		DLLCLIENT int set_skin(lua_State *l);
		DLLCLIENT int skin_exists(lua_State *l);
		DLLCLIENT int get_cursor(lua_State *l);
		DLLCLIENT int set_cursor(lua_State *l);
		DLLCLIENT int get_cursor_input_mode(lua_State *l);
		DLLCLIENT int set_cursor_input_mode(lua_State *l);
		DLLCLIENT int get_window_size(lua_State *l);
		DLLCLIENT int inject_mouse_input(lua_State *l);
		DLLCLIENT int inject_keyboard_input(lua_State *l);
		DLLCLIENT int inject_char_input(lua_State *l);
		DLLCLIENT int inject_scroll_input(lua_State *l);
	};
};

DLLCLIENT int Lua_gui_RealTime(lua_State *l);
DLLCLIENT int Lua_gui_DeltaTime(lua_State *l);
DLLCLIENT int Lua_gui_LastThink(lua_State *l);

#endif