// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/input/input_binding_layer_lua.hpp"

import pragma.client.engine;

extern CEngine *c_engine;

pragma::LuaInputBindingLayerRegister::~LuaInputBindingLayerRegister() { Clear(); }
void pragma::LuaInputBindingLayerRegister::Add(InputBindingLayer &layer) { m_layers.insert(&layer); }
void pragma::LuaInputBindingLayerRegister::Remove(InputBindingLayer &layer)
{
	auto it = m_layers.find(&layer);
	if(it == m_layers.end())
		return;
	c_engine->RemoveInputBindingLayer(layer.identifier);
}
void pragma::LuaInputBindingLayerRegister::Clear()
{
	for(auto &layer : c_engine->GetInputBindingLayers()) {
		auto it = m_layers.find(layer.get());
		if(it == m_layers.end())
			continue;
		c_engine->RemoveInputBindingLayer(layer->identifier);
	}
	m_layers.clear();
}
