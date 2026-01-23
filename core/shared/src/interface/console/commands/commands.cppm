// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.commands;

export import :entities.components.base_player;

export {
	namespace pragma {
		class NetworkState;
	}
	namespace pragma::console::commands {
		DLLNETWORK void lua_run(lua::State *l, const std::string &chunkName, BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_run(NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_run_autocomplete(lua::State *l, const std::string &arg, std::vector<std::string> &autoCompleteOptions);

		DLLNETWORK void lua_exec(NetworkState *state, BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_exec_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions);
	};
};
