// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include <iostream>

import pragma.shared;

std::ostream &operator<<(std::ostream &out, const pragma::BaseEntityComponent *hComponent)
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
