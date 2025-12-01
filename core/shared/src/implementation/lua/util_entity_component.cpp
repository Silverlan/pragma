// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.util_entity_component;

std::ostream &operator<<(std::ostream &out, const pragma::BaseEntityComponent *hComponent)
{
	out << "EntityComponent[";
	if(!hComponent)
		out << "NULL]";
	else {
		auto &ent = hComponent->GetEntity();
		auto &componentInfo = *ent.GetNetworkState()->GetGameState()->GetEntityComponentManager().GetComponentInfo(hComponent->GetComponentId());
		out << componentInfo.name << "][";
		const_cast<pragma::ecs::BaseEntity &>(ent).print(out);
		out << "]";
	}
	return out;
}
