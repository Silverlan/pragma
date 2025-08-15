// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.console.commands;

export namespace pragma::console::commands {
    DLLNETWORK void lua_run(lua_State *l, const std::string &chunkName, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
    DLLNETWORK void lua_run(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
    DLLNETWORK void lua_run_autocomplete(lua_State *l, const std::string &arg, std::vector<std::string> &autoCompleteOptions);

    DLLNETWORK void lua_exec(NetworkState *state, pragma::BasePlayerComponent *, std::vector<std::string> &argv, float);
    DLLNETWORK void lua_exec_autocomplete(const std::string &arg, std::vector<std::string> &autoCompleteOptions);
};
