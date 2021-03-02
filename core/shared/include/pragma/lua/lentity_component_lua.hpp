/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __LENTITY_COMPONWNT_LUA_HPP__
#define __LENTITY_COMPONWNT_LUA_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"
#include <sharedutils/util_weak_handle.hpp>

DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseLuaBaseEntity);

inline std::ostream &operator<<(std::ostream &out,const BaseLuaBaseEntityHandle &hComponent)
{
	out<<"EntityComponent[";
	if(hComponent.expired())
		out<<"NULL]";
	else
	{
		auto &ent = hComponent->GetEntity();
		auto &componentInfo = *ent.GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(hComponent->GetComponentId());
		out<<componentInfo.name<<"][";
		const_cast<BaseEntity&>(ent).print(out);
		out<<"]";
	}
	return out;
}

#endif
