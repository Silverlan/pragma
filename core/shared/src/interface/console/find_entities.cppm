// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string>
#include <vector>
#include <functional>

export module pragma.shared:console.find_entities;

export import :physics.raycast;

export {
	namespace pragma {class Game;}
	namespace pragma::ecs {class BaseEntity;}
	class NetworkState;
	namespace pragma {class BaseCharacterComponent; class BasePlayerComponent;};
	namespace command {
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_target_entity(NetworkState *state, pragma::BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_trace_targets(NetworkState *state, pragma::BaseCharacterComponent &pl, const std::function<void(TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<pragma::ecs::BaseEntity *> find_named_targets(NetworkState *state, const std::string &targetName);
	};

	namespace util {
		namespace cmd {
			DLLNETWORK std::vector<std::pair<pragma::ecs::BaseEntity *, float>> get_sorted_entities(pragma::Game &game, ::pragma::BasePlayerComponent *pl = nullptr);
		};
	};
};
