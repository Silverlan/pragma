// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_GUI_CALLBACKS_HPP__
#define __C_GUI_CALLBACKS_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/ldefinitions.h>

class WIBase;
namespace Lua {
	namespace gui {
		// Note: This has to be a C-function pointer, because some callbacks may be registered
		// from binary modules which are unloaded before the client-module is unloaded, which could cause
		// issues if we were using a cpp object (like std::function) here
		using LUA_CALLBACK = CallbackHandle (*)(::WIBase &, lua_State *, const std::function<void(const std::function<void()> &)> &);
		DLLCLIENT void register_lua_callback(std::string className, std::string callbackName, LUA_CALLBACK fCb);
	};
};

#endif
