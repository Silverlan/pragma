// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "mathutil/glmutil.h"
#include <string>
#include <vector>

#include <cinttypes>
#include <optional>
#include <memory>
#include <functional>
#include <thread>
#include <unordered_map>
#include "pragma/lua/core.hpp"

#include <ostream>

export module pragma.shared:console.commands;

export import :entities.components.base_player;

export {
	class NetworkState;
	namespace pragma::console::commands {
		DLLNETWORK void lua_run(lua_State *l, const std::string &chunkName, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_run(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_run_autocomplete(lua_State *l, const std::string &arg, std::vector<std::string> &autoCompleteOptions);

		DLLNETWORK void lua_exec(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
		DLLNETWORK void lua_exec_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions);
	};
};
