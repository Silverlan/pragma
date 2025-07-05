// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LENTITY_COMPONWNT_LUA_HPP__
#define __LENTITY_COMPONWNT_LUA_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/lua/sh_lua_component.hpp"
#include <sharedutils/util_weak_handle.hpp>

inline std::ostream &operator<<(std::ostream &out, const pragma::BaseEntityComponent *hComponent)
{
	out << "EntityComponent[";
	if(!hComponent)
		out << "NULL]";
	else {
		auto &ent = hComponent->GetEntity();
		auto &componentInfo = *ent.GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(hComponent->GetComponentId());
		out << componentInfo.name << "][";
		const_cast<BaseEntity &>(ent).print(out);
		out << "]";
	}
	return out;
}

#endif
