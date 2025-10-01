// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "sharedutils/util.h"
#include "sharedutils/util_string.h"
#include "mathutil/uvec.h"

module pragma.shared;

import :console.find_entities;

std::vector<BaseEntity *> command::find_trace_targets(NetworkState *state, pragma::BaseCharacterComponent &pl, const std::function<void(TraceData &)> &trCallback)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<BaseEntity *> ents;
	auto trData = pl.GetAimTraceData();
	if(trCallback != nullptr)
		trCallback(trData);
	auto r = game->RayCast(trData);
	if(r.hitType == RayCastHitType::None)
		return ents;
	if(!r.entity.valid() || r.entity->IsWorld())
		return ents;
	ents.push_back(r.entity.get());
	return ents;
}

std::vector<BaseEntity *> command::find_named_targets(NetworkState *state, const std::string &targetName)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<BaseEntity *> ents;
	{
		auto uuid = util::uuid_string_to_bytes(targetName);
		if(uuid != util::Uuid {}) {
			// Check for UUID
			EntityIterator entIt {*game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<EntityIteratorFilterUuid>(uuid);
			ents.reserve(entIt.GetCount());
			for(auto *ent : entIt)
				ents.push_back(ent);
		}
	}
	if(ents.empty()) {
		EntityIterator entIt {*game, EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(targetName);
		ents.reserve(entIt.GetCount());
		for(auto *ent : entIt)
			ents.push_back(ent);
	}
	if(ents.empty()) {
		auto index = ustring::to_int(targetName);
		auto *ent = game->GetEntityByLocalIndex(index);
		if(ent != nullptr)
			ents.push_back(ent);
	}
	return ents;
}

std::vector<BaseEntity *> command::find_target_entity(NetworkState *state, pragma::BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(TraceData &)> &trCallback)
{
	if(argv.empty())
		return find_trace_targets(state, pl, trCallback);
	return find_named_targets(state, argv[0]);
}

std::vector<std::pair<BaseEntity *, float>> util::cmd::get_sorted_entities(Game &game, pragma::BasePlayerComponent *pl)
{
	std::vector<BaseEntity *> *entities;
	game.GetEntities(&entities);

	std::vector<std::pair<BaseEntity *, float>> sortedEntities {};
	auto charComponent = (pl != nullptr) ? pl->GetEntity().GetCharacterComponent() : pragma::ComponentHandle<pragma::BaseCharacterComponent> {};
	sortedEntities.reserve(entities->size());
	for(auto *ent : *entities) {
		if(ent == nullptr)
			continue;
		auto d = std::numeric_limits<float>::max();
		auto trComponent = ent->GetTransformComponent();
		if(trComponent && charComponent.valid())
			d = uvec::distance(charComponent->GetEyePosition(), ent->GetCenter());
		sortedEntities.push_back(std::make_pair(ent, d));
	}
	std::sort(sortedEntities.begin(), sortedEntities.end(), [](const std::pair<BaseEntity *, float> &pair0, const std::pair<BaseEntity *, float> &pair1) { return pair0.second < pair1.second; });
	return sortedEntities;
}
