// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.find_entities;

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_trace_targets(pragma::NetworkState *state, pragma::BaseCharacterComponent &pl, const std::function<void(TraceData &)> &trCallback)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<pragma::ecs::BaseEntity *> ents;
	auto trData = pl.GetAimTraceData();
	if(trCallback != nullptr)
		trCallback(trData);
	auto r = game->RayCast(trData);
	if(r.hitType == pragma::physics::RayCastHitType::None)
		return ents;
	if(!r.entity.valid() || r.entity->IsWorld())
		return ents;
	ents.push_back(r.entity.get());
	return ents;
}

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_named_targets(pragma::NetworkState *state, const std::string &targetName)
{
	auto *game = state->GetGameState();
	if(game == nullptr)
		return {};
	std::vector<pragma::ecs::BaseEntity *> ents;
	{
		auto uuid = util::uuid_string_to_bytes(targetName);
		if(uuid != util::Uuid {}) {
			// Check for UUID
			pragma::ecs::EntityIterator entIt {*game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
			entIt.AttachFilter<EntityIteratorFilterUuid>(uuid);
			ents.reserve(entIt.GetCount());
			for(auto *ent : entIt)
				ents.push_back(ent);
		}
	}
	if(ents.empty()) {
		pragma::ecs::EntityIterator entIt {*game, pragma::ecs::EntityIterator::FilterFlags::Default | pragma::ecs::EntityIterator::FilterFlags::Pending};
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

std::vector<pragma::ecs::BaseEntity *> pragma::console::find_target_entity(pragma::NetworkState *state, pragma::BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(TraceData &)> &trCallback)
{
	if(argv.empty())
		return find_trace_targets(state, pl, trCallback);
	return find_named_targets(state, argv[0]);
}

std::vector<std::pair<pragma::ecs::BaseEntity *, float>> pragma::console::get_sorted_entities(pragma::Game &game, pragma::BasePlayerComponent *pl)
{
	std::vector<pragma::ecs::BaseEntity *> *entities;
	game.GetEntities(&entities);

	std::vector<std::pair<pragma::ecs::BaseEntity *, float>> sortedEntities {};
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
	std::sort(sortedEntities.begin(), sortedEntities.end(), [](const std::pair<pragma::ecs::BaseEntity *, float> &pair0, const std::pair<pragma::ecs::BaseEntity *, float> &pair1) { return pair0.second < pair1.second; });
	return sortedEntities;
}
