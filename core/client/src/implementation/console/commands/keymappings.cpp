// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void CMD_bind_keys(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_bind(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_unbind(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_unbindall(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_keymappings(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
namespace {
	using namespace pragma::console::client;
	auto UVN = register_command("bind_keys", &CMD_bind_keys, pragma::console::ConVarFlags::None, "Prints a list of all bindable keys to the console.");
	auto UVN = register_command("bind", &CMD_bind, pragma::console::ConVarFlags::None, "Binds a key to a command string. Usage: bind <key> <command>");
	auto UVN = register_command("unbind", &CMD_unbind, pragma::console::ConVarFlags::None, "Unbinds the given key.");
	auto UVN = register_command("unbindall", &CMD_unbindall, pragma::console::ConVarFlags::None, "Unbinds all keys.");
	auto UVN = register_command("keymappings", &CMD_keymappings, pragma::console::ConVarFlags::None, "Prints a list of all active key bindings to the console.");
}

////////////////////////////

void CMD_bind_keys(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	for(int i = 0; i < (sizeof(BIND_KEYS) / sizeof(BIND_KEYS[0])); i++)
		Con::COUT << BIND_KEYS[i] << Con::endl;
}

void CMD_bind(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.size() <= 1)
		return;
	short c;
	if(!StringToKey(argv[0], &c)) {
		Con::COUT << "\"" << argv[0] << "\" isn't a valid key. Use 'bind_keys' to get a list of all available keys" << Con::endl;
		return;
	}
	auto bindings = pragma::get_cengine()->GetCoreInputBindingLayer();
	if(bindings)
		bindings->MapKey(c, argv[1]);
	pragma::get_cengine()->SetInputBindingsDirty();
}

void CMD_unbind(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	short c;
	if(!StringToKey(argv[0], &c)) {
		Con::COUT << "\"" << argv[0] << "\" isn't a valid key. Use 'bind_keys' to get a list of all available keys" << Con::endl;
		return;
	}
	auto bindings = pragma::get_cengine()->GetCoreInputBindingLayer();
	if(bindings)
		bindings->UnmapKey(c);
	pragma::get_cengine()->SetInputBindingsDirty();
}

void CMD_unbindall(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto bindings = pragma::get_cengine()->GetCoreInputBindingLayer();
	if(bindings)
		bindings->ClearKeyMappings();
	pragma::get_cengine()->SetInputBindingsDirty();
}

void CMD_keymappings(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &bindings = pragma::get_cengine()->GetEffectiveInputBindingLayer();
	auto &mappings = bindings.GetKeyMappings();
	std::string key;
	for(auto &pair : mappings) {
		if(KeyToString(pair.first, &key)) {
			Con::COUT << key << ": ";
			if(pair.second.GetType() == KeyBind::Type::Regular)
				Con::COUT << "\"" << pair.second.GetBind() << "\"";
			else
				Con::COUT << "function";
			Con::COUT << Con::endl;
		}
	}
}
