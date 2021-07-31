/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_LFACTION_H__
#define __S_LFACTION_H__

#include "pragma/serverdefinitions.h"
#include <pragma/lua/luaapi.h>
#include "pragma/ai/s_factions.h"

namespace Lua
{
	namespace Faction
	{
		DLLSERVER void register_class(lua_State *l,luabind::module_ &mod);
		DLLSERVER void SetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt,uint32_t disposition,bool revert,int32_t priority);
		DLLSERVER void SetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt,uint32_t disposition,bool revert);
		DLLSERVER void SetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt,uint32_t disposition);
		DLLSERVER void SetEnemyFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority);
		DLLSERVER void SetEnemyFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert);
		DLLSERVER void SetEnemyFaction(lua_State *l,::Faction &faction,::Faction &factionTgt);
		DLLSERVER void SetAlliedFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority);
		DLLSERVER void SetAlliedFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert);
		DLLSERVER void SetAlliedFaction(lua_State *l,::Faction &faction,::Faction &factionTgt);
		DLLSERVER void SetNeutralFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority);
		DLLSERVER void SetNeutralFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert);
		DLLSERVER void SetNeutralFaction(lua_State *l,::Faction &faction,::Faction &factionTgt);
		DLLSERVER void SetFearsomeFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert,int32_t priority);
		DLLSERVER void SetFearsomeFaction(lua_State *l,::Faction &faction,::Faction &factionTgt,bool revert);
		DLLSERVER void SetFearsomeFaction(lua_State *l,::Faction &faction,::Faction &factionTgt);
		DLLSERVER void GetDisposition(lua_State *l,::Faction &faction,::Faction &factionTgt);
		DLLSERVER void GetDisposition(lua_State *l,::Faction &faction,const std::string &className);
		DLLSERVER void GetDisposition(lua_State *l,::Faction &faction,BaseEntity &ent);
	};
};

#endif
