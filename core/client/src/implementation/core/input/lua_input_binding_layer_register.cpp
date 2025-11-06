// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :core.lua_input_binding_layer_register;
import :engine;

pragma::LuaCoreInputBindingLayerRegister::~LuaInputBindingLayerRegister() { Clear(); }
void pragma::LuaCoreInputBindingLayerRegister::Add(InputBindingLayer &layer) { m_layers.insert(&layer); }
void pragma::LuaCoreInputBindingLayerRegister::Remove(InputBindingLayer &layer)
{
	auto it = m_layers.find(&layer);
	if(it == m_layers.end())
		return;
	pragma::get_cengine()->RemoveInputBindingLayer(layer.identifier);
}
void pragma::LuaCoreInputBindingLayerRegister::Clear()
{
	for(auto &layer : pragma::get_cengine()->GetInputBindingLayers()) {
		auto it = m_layers.find(layer.get());
		if(it == m_layers.end())
			continue;
		pragma::get_cengine()->RemoveInputBindingLayer(layer->identifier);
	}
	m_layers.clear();
}
