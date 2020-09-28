/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LDEF_WGUI_H__
#define __C_LDEF_WGUI_H__

#include "pragma/lua/classes/c_ldef_wguihandles.h"

LUA_SETUP_HANDLE_CHECK(GUIElement,::WIBase,WIHandle);

#define lua_checkgui_ret(l,hGUI,RET) \
	if(!hGUI.IsValid()) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL GUI Element"); \
		lua_error(l); \
		return RET; \
	}

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