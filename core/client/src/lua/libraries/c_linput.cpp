/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/lua/libraries/c_linput.h"
#include "pragma/localization.h"
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/input/inputhelper.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

int Lua::input::get_mouse_button_state(lua_State *l)
{
	auto mouseButton = Lua::CheckInt(l,1);
	auto &window = c_engine->GetWindow();
	Lua::PushInt(l,window.GetMouseButtonState(static_cast<GLFW::MouseButton>(mouseButton)));
	return 1;
}
int Lua::input::get_key_state(lua_State *l)
{
	auto key = Lua::CheckInt(l,1);
	auto &window = c_engine->GetWindow();
	Lua::PushInt(l,window.GetKeyState(static_cast<GLFW::Key>(key)));
	return 1;
}
int Lua::input::add_callback(lua_State *l)
{
	auto *identifier = Lua::CheckString(l,1);
	Lua::CheckFunction(l,2);

	auto &inputHandler = c_game->GetInputCallbackHandler();
	auto hCallback = inputHandler.AddLuaCallback(identifier,luabind::object(luabind::from_stack(l,2)));
	Lua::Push<CallbackHandle>(l,hCallback);
	return 1;
}
int Lua::input::get_cursor_pos(lua_State *l)
{
	auto &window = c_engine->GetWindow();
	auto pos = window.GetCursorPos();
	Lua::Push<Vector2>(l,pos);
	return 1;
}
int Lua::input::get_controller_count(lua_State *l)
{
	Lua::PushInt(l,GLFW::get_joysticks().size());
	return 1;
}
int Lua::input::get_controller_name(lua_State *l)
{
	auto joystickId = Lua::CheckInt(l,1);
	Lua::PushString(l,GLFW::get_joystick_name(joystickId));
	return 1;
}
int Lua::input::get_joystick_axes(lua_State *l)
{
	auto joystickId = Lua::CheckInt(l,1);
	auto &axes = GLFW::get_joystick_axes(joystickId);
	auto t = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto axis : axes)
	{
		Lua::PushInt(l,idx++);
		Lua::PushNumber(l,axis);
		Lua::SetTableValue(l,t);
	}
	return 1;
}
int Lua::input::get_joystick_buttons(lua_State *l)
{
	auto joystickId = Lua::CheckInt(l,1);
	auto &buttons = GLFW::get_joystick_buttons(joystickId);
	auto t = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto bt : buttons)
	{
		Lua::PushInt(l,idx++);
		Lua::PushInt(l,umath::to_integral(bt));
		Lua::SetTableValue(l,t);
	}
	return 1;
}

int Lua::input::set_cursor_pos(lua_State *l)
{
	auto &cursorPos = *Lua::CheckVector2(l,1);
	auto &window = c_engine->GetWindow();
	window.SetCursorPos(cursorPos);
	return 0;
}

int Lua::input::key_to_string(lua_State *l)
{
	auto key = Lua::CheckInt(l,1);
	std::string str;
	auto b = KeyToString(key,&str);
	Lua::PushBool(l,b);
	if(b == true)
	{
		Lua::PushString(l,str);
		return 2;
	}
	return 1;
}
int Lua::input::key_to_text(lua_State *l)
{
	auto key = Lua::CheckInt(l,1);
	std::string str;
	auto b = KeyToText(key,&str);
	Lua::PushBool(l,b);
	if(b == true)
	{
		Lua::PushString(l,str);
		return 2;
	}
	return 1;
}
int Lua::input::string_to_key(lua_State *l)
{
	std::string str = Lua::CheckString(l,1);
	auto key = short{};
	auto b = StringToKey(str,&key);
	Lua::PushBool(l,b);
	if(b == true)
	{
		Lua::PushInt(l,key);
		return 2;
	}
	return 1;
}
int Lua::input::get_mapped_keys(lua_State *l)
{
	std::string cvarName = Lua::CheckString(l,1);
	std::vector<GLFW::Key> mappedKeys {};
	auto maxKeys = std::numeric_limits<uint32_t>::max();
	if(Lua::IsSet(l,2))
		maxKeys = Lua::CheckInt(l,2);
	c_engine->GetMappedKeys(cvarName,mappedKeys,maxKeys);

	auto t = Lua::CreateTable(l);
	int32_t keyIdx = 1;
	for(auto &key : mappedKeys)
	{
		Lua::PushInt(l,keyIdx++);
		Lua::PushInt(l,umath::to_integral(key));
		Lua::SetTableValue(l,t);
	}
	return 1;
}
