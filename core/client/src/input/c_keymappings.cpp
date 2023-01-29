/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/c_engine.h"
#include "pragma/input/inputhelper.h"
#include "pragma/input/input_binding_layer.hpp"
#include <pragma/clientstate/clientstate.h>

void CEngine::AddInputBindingLayer(const std::shared_ptr<InputBindingLayer> &layer)
{
	RemoveInputBindingLayer(layer->identifier);
	m_inputBindingLayers.push_back(layer);
	std::sort(m_inputBindingLayers.begin(), m_inputBindingLayers.end(), [](const std::shared_ptr<InputBindingLayer> &a, const std::shared_ptr<InputBindingLayer> &b) { return a->priority < b->priority; });
	SetInputBindingsDirty();
}
std::vector<std::shared_ptr<InputBindingLayer>> CEngine::GetInputBindingLayers() { return m_inputBindingLayers; }
std::shared_ptr<InputBindingLayer> CEngine::GetInputBindingLayer(const std::string &name)
{
	auto it = std::find_if(m_inputBindingLayers.begin(), m_inputBindingLayers.end(), [&name](const std::shared_ptr<InputBindingLayer> &layer) { return layer->identifier == name; });
	return (it != m_inputBindingLayers.end()) ? *it : nullptr;
}
bool CEngine::RemoveInputBindingLayer(const std::string &name)
{
	auto it = std::find_if(m_inputBindingLayers.begin(), m_inputBindingLayers.end(), [&name](const std::shared_ptr<InputBindingLayer> &layer) { return layer->identifier == name; });
	if(it == m_inputBindingLayers.end())
		return false;
	m_inputBindingLayers.erase(it);
	SetInputBindingsDirty();
	return true;
}
void CEngine::UpdateDirtyInputBindings()
{
	if(!umath::is_flag_set(m_stateFlags, StateFlags::InputBindingsDirty))
		return;
	umath::set_flag(m_stateFlags, StateFlags::InputBindingsDirty, false);
	auto &keybinds = m_coreInputBindingLayer->GetKeyMappings();
	keybinds.clear();
	for(auto &layer : m_inputBindingLayers) {
		if(!layer->enabled)
			continue;
		for(auto &pair : layer->GetKeyMappings())
			keybinds[pair.first] = pair.second;
	}
}

void CEngine::SetInputBindingsDirty() { umath::set_flag(m_stateFlags, StateFlags::InputBindingsDirty); }

const InputBindingLayer &CEngine::GetEffectiveInputBindingLayer() { return *m_coreInputBindingLayer; }

std::shared_ptr<InputBindingLayer> CEngine::GetCoreInputBindingLayer() { return GetInputBindingLayer("core"); }

////////////////////////////

#include "pragma/console/c_cvar_keymappings.h"
extern CEngine *c_engine;
DLLCLIENT void CMD_bind_keys(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	for(int i = 0; i < (sizeof(BIND_KEYS) / sizeof(BIND_KEYS[0])); i++)
		Con::cout << BIND_KEYS[i] << Con::endl;
}

DLLCLIENT void CMD_bind(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.size() <= 1)
		return;
	short c;
	if(!StringToKey(argv[0], &c)) {
		Con::cout << "\"" << argv[0] << "\" isn't a valid key. Use 'bind_keys' to get a list of all available keys" << Con::endl;
		return;
	}
	auto bindings = c_engine->GetCoreInputBindingLayer();
	if(bindings)
		bindings->MapKey(c, argv[1]);
	c_engine->SetInputBindingsDirty();
}

DLLCLIENT void CMD_unbind(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	short c;
	if(!StringToKey(argv[0], &c)) {
		Con::cout << "\"" << argv[0] << "\" isn't a valid key. Use 'bind_keys' to get a list of all available keys" << Con::endl;
		return;
	}
	auto bindings = c_engine->GetCoreInputBindingLayer();
	if(bindings)
		bindings->UnmapKey(c);
	c_engine->SetInputBindingsDirty();
}

DLLCLIENT void CMD_unbindall(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto bindings = c_engine->GetCoreInputBindingLayer();
	if(bindings)
		bindings->ClearKeyMappings();
	c_engine->SetInputBindingsDirty();
}

DLLCLIENT void CMD_keymappings(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &bindings = c_engine->GetEffectiveInputBindingLayer();
	auto &mappings = bindings.GetKeyMappings();
	std::string key;
	for(auto &pair : mappings) {
		if(KeyToString(pair.first, &key)) {
			Con::cout << key << ": ";
			if(pair.second.GetType() == KeyBind::Type::Regular)
				Con::cout << "\"" << pair.second.GetBind() << "\"";
			else
				Con::cout << "function";
			Con::cout << Con::endl;
		}
	}
}
