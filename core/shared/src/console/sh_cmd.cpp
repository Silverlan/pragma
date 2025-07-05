// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include "pragma/entities/baseentity.h"
#include "pragma/console/sh_cmd.h"
#include "pragma/physics/raytraces.h"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/entity_iterator.hpp"

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
