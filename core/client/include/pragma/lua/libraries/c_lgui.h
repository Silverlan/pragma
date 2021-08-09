/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef _C_LGUI_H__
#define _C_LGUI_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

namespace Lua
{
	namespace gui
	{
		DLLCLIENT ::WIBase *create(CGame *game,const std::string &name,::WIBase &parent,int32_t x,int32_t y,uint32_t w,uint32_t h,float left,float top,float right,float bottom);
		DLLCLIENT ::WIBase *create(CGame *game,const std::string &name,::WIBase &parent,int32_t x,int32_t y,uint32_t w,uint32_t h);
		DLLCLIENT ::WIBase *create(CGame *game,const std::string &name,::WIBase &parent,int32_t x,int32_t y);
		DLLCLIENT ::WIBase *create(CGame *game,const std::string &name,::WIBase *parent);
		DLLCLIENT ::WIBase *create(CGame *game,const std::string &name);

		DLLCLIENT ::WIBase *create_button(lua_State *l,const std::string &text);
		DLLCLIENT ::WIBase *create_button(lua_State *l,const std::string &text,::WIBase &parent);
		DLLCLIENT ::WIBase *create_button(lua_State *l,const std::string &text,::WIBase &parent,int32_t x,int32_t y);

		DLLCLIENT Lua::opt<Lua::mult<Lua::type<::WIBase>,Lua::type<::WIBase>,Lua::type<::WIBase>>> create_checkbox(lua_State *l,const std::string &label,::WIBase &parent);
		DLLCLIENT Lua::opt<Lua::mult<Lua::type<::WIBase>,Lua::type<::WIBase>,Lua::type<::WIBase>>> create_checkbox(lua_State *l,const std::string &label);

		DLLCLIENT ::WIBase *create_label(lua_State *l,const std::string &str,::WIBase &parent,int32_t x,int32_t y);
		DLLCLIENT ::WIBase *create_label(lua_State *l,const std::string &str,::WIBase &parent);
		DLLCLIENT ::WIBase *create_label(lua_State *l,const std::string &str);

		DLLCLIENT void register_element(const std::string &className,const Lua::classObject &classData);
		DLLCLIENT ::WIBase *get_base_element(const prosper::Window &window);
		DLLCLIENT ::WIBase *get_base_element();

		DLLCLIENT ::WIBase *get_element_at_position(lua_State *l);
		DLLCLIENT ::WIBase *get_element_at_position(lua_State *l,prosper::Window *window);
		DLLCLIENT ::WIBase *get_element_at_position(lua_State *l,prosper::Window *window,::WIBase *baseElement);
		DLLCLIENT ::WIBase *get_element_at_position(lua_State *l,prosper::Window *window,::WIBase *baseElement,int32_t x,int32_t y);
		DLLCLIENT ::WIBase *get_element_at_position(lua_State *l,prosper::Window *window,::WIBase *baseElement,int32_t x,int32_t y,const Lua::func<bool,::WIBase> &condition);
		
		DLLCLIENT ::WIBase *get_element_under_cursor(lua_State *l,const prosper::Window *window,const Lua::func<bool,::WIBase> &condition);
		DLLCLIENT ::WIBase *get_element_under_cursor(lua_State *l,const Lua::func<bool,::WIBase> &condition);
		DLLCLIENT ::WIBase *get_element_under_cursor(lua_State *l,const prosper::Window *window=nullptr);
		DLLCLIENT ::WIBase *get_focused_element(lua_State *l);
		DLLCLIENT bool register_skin(lua_State *l,const std::string &skin,const luabind::tableT<void> &vars,const luabind::tableT<void> &skinData);
		DLLCLIENT bool register_skin(lua_State *l,const std::string &skin,const luabind::tableT<void> &vars,const luabind::tableT<void> &skinData,const std::string &baseName);
		DLLCLIENT bool load_skin(const std::string &skinName);
		DLLCLIENT void set_skin(const std::string &skin);
		DLLCLIENT bool skin_exists(const std::string &name);
		DLLCLIENT GLFW::Cursor::Shape get_cursor();
		DLLCLIENT void set_cursor(GLFW::Cursor::Shape shape);
		DLLCLIENT GLFW::CursorMode get_cursor_input_mode();
		DLLCLIENT void set_cursor_input_mode(GLFW::CursorMode mode);
		DLLCLIENT ::Vector2i get_window_size(lua_State *l);
		DLLCLIENT bool inject_mouse_input(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods,const Vector2i &pCursorPos);
		DLLCLIENT bool inject_mouse_input(GLFW::MouseButton button,GLFW::KeyState state,GLFW::Modifier mods);
		DLLCLIENT bool inject_keyboard_input(GLFW::Key key,GLFW::KeyState state,GLFW::Modifier mods);
		DLLCLIENT bool inject_char_input(const std::string &c);
		DLLCLIENT bool inject_scroll_input(lua_State *l,const Vector2 &offset,const ::Vector2i &pCursorPos);
		DLLCLIENT bool inject_scroll_input(lua_State *l,const Vector2 &offset);

		DLLCLIENT float RealTime(lua_State *l);
		DLLCLIENT float DeltaTime(lua_State *l);
		DLLCLIENT float LastThink(lua_State *l);
	};
};

#endif