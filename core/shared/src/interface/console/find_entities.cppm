// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <string>

export module pragma.shared:console.find_entities;

export {
	namespace command {
		DLLNETWORK std::vector<BaseEntity *> find_target_entity(NetworkState *state, pragma::BaseCharacterComponent &pl, std::vector<std::string> &argv, const std::function<void(TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<BaseEntity *> find_trace_targets(NetworkState *state, pragma::BaseCharacterComponent &pl, const std::function<void(TraceData &)> &trCallback = nullptr);
		DLLNETWORK std::vector<BaseEntity *> find_named_targets(NetworkState *state, const std::string &targetName);
	};

	namespace util {
		namespace cmd {
			DLLNETWORK std::vector<std::pair<BaseEntity *, float>> get_sorted_entities(Game &game, ::pragma::BasePlayerComponent *pl = nullptr);
		};
	};
};
