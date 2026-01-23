// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :core.lua_input_binding_layer_register;
import :engine;

pragma::LuaInputBindingLayerRegister::~LuaInputBindingLayerRegister() { Clear(); }
void pragma::LuaInputBindingLayerRegister::Add(InputBindingLayer &layer) { m_layers.insert(&layer); }
void pragma::LuaInputBindingLayerRegister::Remove(InputBindingLayer &layer)
{
	auto it = m_layers.find(&layer);
	if(it == m_layers.end())
		return;
	get_cengine()->RemoveInputBindingLayer(layer.identifier);
}
void pragma::LuaInputBindingLayerRegister::Clear()
{
	for(auto &layer : get_cengine()->GetInputBindingLayers()) {
		auto it = m_layers.find(layer.get());
		if(it == m_layers.end())
			continue;
		get_cengine()->RemoveInputBindingLayer(layer->identifier);
	}
	m_layers.clear();
}
