/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/classes/lcallback.h"
#include "pragma/lua/luacallback.h"
#include "luasystem.h"
#include <sharedutils/util_string.h>
#include "pragma/lua/lua_call.hpp"

static std::ostream &operator<<(std::ostream &out, const CallbackHandler &)
{
	out << "CallbackHandler";
	return out;
}

static bool operator==(const CallbackHandler &a, const CallbackHandler &b) { return (&a == &b) ? true : false; }

void Lua::CallbackHandler::register_class(luabind::class_<::CallbackHandler> &classDef)
{
	classDef.def(luabind::constructor<>());
	classDef.def(luabind::tostring(luabind::self));
	classDef.def(luabind::const_self == luabind::const_self);
	classDef.def("AddCallback", &AddCallback);
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
	classDef.def("CallCallbacks", static_cast<void (*)(lua_State *, ::CallbackHandler &, const std::string &, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object)>(&CallCallbacks));
}

namespace Lua {
	namespace CallbackHandler {
		template<typename... TARGS>
		void CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, std::string name, TARGS... args)
		{
			ustring::to_lower(name);
			auto *callbacks = cbHandler.GetCallbacks(name);
			if(callbacks == nullptr)
				return;
			uint32_t argOffset = 3;
			auto numArgs = Lua::GetStackTop(l) - argOffset + 1;
			for(auto it = callbacks->begin(); it != callbacks->end();) {
				auto &hCb = *it;
				if(!hCb.IsValid())
					it = callbacks->erase(it);
				else {
					auto *cb = static_cast<LuaCallback *>(hCb.get());
					auto &o = cb->GetLuaObject();
					Lua::CallFunction(
					  l,
					  [&o, &cbHandler, numArgs, argOffset](lua_State *l) {
						  o.push(l);
						  for(auto i = decltype(numArgs) {0}; i < numArgs; ++i) {
							  auto arg = argOffset + i;
							  Lua::PushValue(l, arg);
						  }
						  return Lua::StatusCode::Ok;
					  },
					  0);
					++it;
				}
			}
		}
	};
};
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name) { CallCallbacks<>(l, cbHandler, name); }
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1) { CallCallbacks<luabind::object>(l, cbHandler, name, o1); }
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2) { CallCallbacks<luabind::object, luabind::object>(l, cbHandler, name, o1, o2); }
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3) { CallCallbacks<luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3); }
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3, o4);
}
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3, o4, o5);
}
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3, o4, o5, o6);
}
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3, o4, o5, o6, o7);
}
void Lua::CallbackHandler::CallCallbacks(lua_State *l, ::CallbackHandler &cbHandler, const std::string &name, luabind::object o1, luabind::object o2, luabind::object o3, luabind::object o4, luabind::object o5, luabind::object o6, luabind::object o7, luabind::object o8)
{
	CallCallbacks<luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object, luabind::object>(l, cbHandler, name, o1, o2, o3, o4, o5, o6, o7, o8);
}

void Lua::CallbackHandler::AddCallback(lua_State *l, ::CallbackHandler &cbHandler, std::string name, luabind::object o)
{
	Lua::CheckFunction(l, 3);

	ustring::to_lower(name);
	auto hCallback = cbHandler.AddCallback(name, CallbackHandle {std::shared_ptr<TCallback>(new LuaCallback(o))});

	Lua::Push<CallbackHandle>(l, hCallback);
}

DLLNETWORK void Lua_Callback_IsValid(lua_State *l, CallbackHandle &callback) { Lua::PushBool(l, callback.IsValid()); }

DLLNETWORK void Lua_Callback_Remove(lua_State *, CallbackHandle &callback) { callback.Remove(); }
