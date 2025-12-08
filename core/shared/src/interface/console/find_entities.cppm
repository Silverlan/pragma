// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.find_entities;

export import :physics.raycast;

export {
	namespace pragma {
		class Game;
	}
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma {
		class BaseCharacterComponent;
		class BasePlayerComponent;
		class NetworkState;
	};
	namespace pragma::console {
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_target_entity(pragma::NetworkState *state, pragma::BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(pragma::physics::TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_trace_targets(pragma::NetworkState *state, pragma::BaseCharacterComponent &pl, const std::function<void(pragma::physics::TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_named_targets(pragma::NetworkState *state, const std::string &targetName);

		DLLNETWORK std::vector<std::pair<pragma::ecs::BaseEntity *, float>> get_sorted_entities(pragma::Game &game, ::pragma::BasePlayerComponent *pl = nullptr);
	};
};
