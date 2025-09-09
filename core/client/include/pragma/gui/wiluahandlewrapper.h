// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __WILUAHANDLEWRAPPER_H__
#define __WILUAHANDLEWRAPPER_H__

#include "pragma/clientdefinitions.h"
#include <luabind/luabind.hpp>
#include <wgui/types.hpp>

class DLLCLIENT WILuaHandleWrapper {
  private:
	WILuaHandleWrapper() {}
  public:
	template<class THandle>
	static WILuaHandleWrapper *Create(lua_State *l, THandle handle)
	{
		WILuaHandleWrapper *wrapper = new WILuaHandleWrapper;
		wrapper->object = new luabind::object(l, handle);
		wrapper->handle = handle;
		wrapper->lua = false;
		return wrapper;
	}
	~WILuaHandleWrapper()
	{
		delete object;
		delete handle;
	}
	luabind::object *object;
	WIHandle *handle;
	bool lua; // Has this been created through lua? This is used to clean up all lua-created GUI Elements when the game ends
};

#endif
