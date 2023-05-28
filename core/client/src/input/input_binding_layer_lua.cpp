/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/input/input_binding_layer.hpp"
#include "pragma/input/input_binding_layer_lua.hpp"

extern DLLCLIENT CEngine *c_engine;

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
