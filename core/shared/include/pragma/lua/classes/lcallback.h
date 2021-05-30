/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __LCALLBACK_H__
#define __LCALLBACK_H__
#include "pragma/networkdefinitions.h"
#include <sharedutils/functioncallback.h>
#include <pragma/lua/luaapi.h>

namespace Lua
{
	namespace CallbackHandler
	{
		DLLNETWORK void register_class(luabind::class_<::CallbackHandler> &classDef);
		DLLNETWORK void AddCallback(lua_State *l,::CallbackHandler &cbHandler,std::string name,luabind::object o);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7);
		DLLNETWORK void CallCallbacks(lua_State *l,::CallbackHandler &cbHandler,const std::string &name,luabind::object o1,luabind::object o2,luabind::object o3,luabind::object o4,luabind::object o5,luabind::object o6,luabind::object o7,luabind::object o8);
	};
};



DLLNETWORK void Lua_Callback_IsValid(lua_State *l,CallbackHandle &callback);
DLLNETWORK void Lua_Callback_Remove(lua_State *l,CallbackHandle &callback);

#endif