// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LDEF_ENTITY_H__
#define __LDEF_ENTITY_H__
#include "pragma/lua/ldefinitions.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseentity_handle.h"

/*#define LUA_CHECK_ENTITY_RET(l,hEnt,ret) \
	if(hEnt == nullptr) \
	{ \
		lua_pushstring(l,"Attempted to use a NULL entity"); \
		lua_error(l); \
		return ret; \
	}

#define LUA_CHECK_ENTITY(l,hEnt) \
	LUA_CHECK_ENTITY_RET(l,hEnt,);

LUA_SETUP_HANDLE_CHECK(Entity,BaseEntity,EntityHandle);
*/
namespace Lua {
	DLLNETWORK bool is_entity(const luabind::object &o);
};

#endif
