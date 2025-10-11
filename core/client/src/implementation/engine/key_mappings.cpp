// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"

#include "mathutil/umath.h"

#include "stdafx_cengine.h"

module pragma.client;


import :engine;
import :client_state;

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
