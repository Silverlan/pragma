// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

DLLCLIENT void CMD_bind_keys(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_bind(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_unbind(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_unbindall(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_keymappings(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
namespace {
	using namespace pragma::console::client;
	auto UVN = register_command("bind_keys", &CMD_bind_keys, ConVarFlags::None, "Prints a list of all bindable keys to the console.");
	auto UVN = register_command("bind", &CMD_bind, ConVarFlags::None, "Binds a key to a command string. Usage: bind <key> <command>");
	auto UVN = register_command("unbind", &CMD_unbind, ConVarFlags::None, "Unbinds the given key.");
	auto UVN = register_command("unbindall", &CMD_unbindall, ConVarFlags::None, "Unbinds all keys.");
	auto UVN = register_command("keymappings", &CMD_keymappings, ConVarFlags::None, "Prints a list of all active key bindings to the console.");
}
