// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.layers;

using namespace pragma::rendering;

LayerManager::LayerManager()
{
	std::fill(m_flagMasks.begin(), m_flagMasks.end(), ALL_LAYERS);
	for(size_t i = 0; i < math::to_integral(StandardLayer::Count); ++i) {
		auto mask = 1 << i;
		auto layerFlags = LayerFlags::Default;
		auto layer = static_cast<StandardLayer>(mask);
		if(layer == StandardLayer::Gizmo || layer == StandardLayer::Helper)
			layerFlags &= ~LayerFlags::CastShadows;
		RegisterLayer(get_standard_layer_name(layer), layerFlags);
	}
}

void LayerManager::UnregisterLayer(std::string_view name)
{
	auto it = m_layers.find(name);
	if(it != m_layers.end())
		return;
	auto mask = it->second;
	m_layers.erase(it);
	m_freeMasks.push(mask);
	for(auto &flagMask : m_flagMasks)
		math::set_flag(flagMask, mask, true);
}
std::optional<LayerMask> LayerManager::RegisterLayer(std::string_view name, LayerFlags flags)
{
	auto it = m_layers.find(name);
	if(it != m_layers.end())
		return it->second;

	LayerMask mask;
	if(!m_freeMasks.empty()) {
		mask = m_freeMasks.front();
		m_freeMasks.pop();
	}
	else {
		if(m_nextBitIndex >= std::numeric_limits<LayerMask>::digits)
			return {}; // All slots are taken
		auto bitIndex = m_nextBitIndex++;
		mask = layer_to_mask(bitIndex);
	}

	m_layers[std::string {name}] = mask;
	for(size_t i = 0; i < math::to_integral(LayerFlag::Count); ++i)
		math::set_flag(m_flagMasks[i], mask, math::is_flag_set(flags, static_cast<LayerFlags>(1 << i)));
	return mask;
}

LayerMask LayerManager::GetMask(std::string_view name) const
{
	auto it = m_layers.find(name);
	if(it != m_layers.end())
		return it->second;
	return DEFAULT_LAYER;
}

bool LayerManager::ShouldPass(LayerMask mask, LayerFlag flag) const { return (m_flagMasks[math::to_integral(flag)] & mask) != 0; }

const pragma::string::StringMap<LayerMask> &LayerManager::GetRegisteredLayers() const { return m_layers; }
