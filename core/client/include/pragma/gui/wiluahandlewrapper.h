/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __WILUAHANDLEWRAPPER_H__
#define __WILUAHANDLEWRAPPER_H__

#include "pragma/clientdefinitions.h"

class DLLCLIENT WILuaHandleWrapper
{
private:
	WILuaHandleWrapper()
	{}
public:
	template<class THandle>
		static WILuaHandleWrapper *Create(lua_State *l,THandle handle)
	{
		WILuaHandleWrapper *wrapper = new WILuaHandleWrapper;
		wrapper->object = new luabind::object(l,handle);
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