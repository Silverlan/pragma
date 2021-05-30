/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/console/util_cmd.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"

std::vector<std::pair<BaseEntity*,float>> util::cmd::get_sorted_entities(Game &game,pragma::BasePlayerComponent *pl)
{
	std::vector<BaseEntity*> *entities;
	game.GetEntities(&entities);

	std::vector<std::pair<BaseEntity*,float>> sortedEntities {};
	auto charComponent = (pl != nullptr) ? pl->GetEntity().GetCharacterComponent() : util::WeakHandle<pragma::BaseCharacterComponent>{};
	sortedEntities.reserve(entities->size());
	for(auto *ent : *entities)
	{
		if(ent == nullptr)
			continue;
		auto d = std::numeric_limits<float>::max();
		auto trComponent = ent->GetTransformComponent();
		if(trComponent && charComponent.valid())
			d = uvec::distance(charComponent->GetEyePosition(),ent->GetCenter());
		sortedEntities.push_back(std::make_pair(ent,d));
	}
	std::sort(sortedEntities.begin(),sortedEntities.end(),[](const std::pair<BaseEntity*,float> &pair0,const std::pair<BaseEntity*,float> &pair1) {
		return pair0.second < pair1.second;
	});
	return sortedEntities;
}
