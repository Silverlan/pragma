// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.find_entities;

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_trace_targets(NetworkState *state, BaseCharacterComponent &pl, const std::function<void(physics::TraceData &)> &trCallback)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<ecs::BaseEntity *> ents;
	auto trData = pl.GetAimTraceData();
	if(trCallback != nullptr)
		trCallback(trData);
	auto r = game->RayCast(trData);
	if(r.hitType == physics::RayCastHitType::None)
		return ents;
	if(!r.entity.valid() || r.entity->IsWorld())
		return ents;
	ents.push_back(r.entity.get());
	return ents;
}

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_named_targets(NetworkState *state, const std::string &targetName)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<ecs::BaseEntity *> ents;
	{
		auto uuid = util::uuid_string_to_bytes(targetName);
		if(uuid != util::Uuid {}) {
			// Check for UUID
			ecs::EntityIterator entIt {*game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<EntityIteratorFilterUuid>(uuid);
			ents.reserve(entIt.GetCount());
			for(auto *ent : entIt)
				ents.push_back(ent);
		}
	}
	if(ents.empty()) {
		ecs::EntityIterator entIt {*game, ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(targetName);
		ents.reserve(entIt.GetCount());
		for(auto *ent : entIt)
			ents.push_back(ent);
	}
	if(ents.empty()) {
		auto index = string::to_int(targetName);
		auto *ent = game->GetEntityByLocalIndex(index);
		if(ent != nullptr)
			ents.push_back(ent);
	}
	return ents;
}

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_target_entity(NetworkState *state, BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(physics::TraceData &)> &trCallback)
{
	if(argv.empty())
		return find_trace_targets(state, pl, trCallback);
	return find_named_targets(state, argv[0]);
}

std::vector<std::pair<pragma::ecs::BaseEntity *, float>> pragma::console::get_sorted_entities(Game &game, BasePlayerComponent *pl)
{
	std::vector<ecs::BaseEntity *> *entities;
	game.GetEntities(&entities);

	std::vector<std::pair<ecs::BaseEntity *, float>> sortedEntities {};
	auto charComponent = (pl != nullptr) ? pl->GetEntity().GetCharacterComponent() : pragma::ComponentHandle<BaseCharacterComponent> {};
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
	std::sort(sortedEntities.begin(), sortedEntities.end(), [](const std::pair<ecs::BaseEntity *, float> &pair0, const std::pair<ecs::BaseEntity *, float> &pair1) { return pair0.second < pair1.second; });
	return sortedEntities;
}
