// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.callback;

export import pragma.lua;

export {
	namespace Lua {
		namespace CallbackHandler {
			DLLNETWORK void register_class(luabind::class_<pragma::util::CallbackHandler> &classDef);
			DLLNETWORK void AddCallback(lua::State *l, pragma::util::CallbackHandler &cbHandler, std::string name, luabind::object o);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7);
			DLLNETWORK void CallCallbacks(lua::State *l, pragma::util::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8);
		};
	};

	DLLNETWORK void Lua_Callback_IsValid(lua::State *l, CallbackHandle &callback);
	DLLNETWORK void Lua_Callback_Remove(lua::State *l, CallbackHandle &callback);
};
