// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "util_enum_flags.hpp"

export module pragma.client:rendering.layers;

export import pragma.math;
import pragma.string;

export namespace pragma::rendering {
	// Note: Scene index is *not* unique, a child-scene will share its index with its parent!
	using SceneIndex = uint8_t;
	using SceneFlags = uint32_t;
	using SceneFlag = uint32_t;
	constexpr auto ALL_SCENES = std::numeric_limits<SceneFlags>::max();

	DLLCLIENT SceneFlags get_scene_flag(SceneIndex sceneIndex) { return 1 << sceneIndex; }
	DLLCLIENT SceneIndex get_scene_index(SceneFlags flag) { return math::get_least_significant_set_bit_index(flag); }

	DLLCLIENT void add_to_scene(SceneFlags &flags, SceneIndex scene) { flags |= get_scene_flag(scene); }
	DLLCLIENT void remove_from_scene(SceneFlags &flags, SceneIndex scene) { flags &= ~get_scene_flag(scene); }
	DLLCLIENT void remove_from_all_scenes(SceneFlags &flags) { flags = 0; }
	DLLCLIENT bool is_in_scene(SceneFlags flags, SceneIndex scene) { return (flags & get_scene_flag(scene)) != 0; }

	// Note: We're using 32 bits because lua's bit library does not work with 64 bit integers
	using LayerMask = uint32_t;
	using VisibilityMask = uint32_t;
	using LayerIndex = uint8_t;
	enum class StandardLayer : LayerMask {
		Default = 1,
		Helper = Default << 1, // e.g. entity icons, visible trigger meshes, etc.
		Gizmo = Helper << 1,   // e.g. transform arrows, visible camera frustum, etc.

		Count = 3,
	};
	const char *get_standard_layer_name(StandardLayer layer)
	{
		switch(layer) {
		case StandardLayer::Default:
			return "default";
		case StandardLayer::Helper:
			return "helper";
		case StandardLayer::Gizmo:
			return "gizmo";
		}
		static_assert(math::to_integral(StandardLayer::Count) == 3, "Update the switch case above when new standard layers are added!");
		std::unreachable();
		return nullptr;
	}
	constexpr auto DEFAULT_LAYER = math::to_integral(StandardLayer::Default);
	const VisibilityMask ALL_LAYERS = std::numeric_limits<VisibilityMask>::max();
	const VisibilityMask NO_LAYERS = 0;
	DLLCLIENT LayerMask layer_to_mask(LayerIndex layer) { return 1 << layer; }
	DLLCLIENT bool is_layer_visible(VisibilityMask mask, LayerMask objectLayer) { return (mask & objectLayer) != 0; }
	DLLCLIENT void enable_layer(VisibilityMask &mask, LayerMask layer) { mask |= layer; }
	DLLCLIENT void disable_layer(VisibilityMask &mask, LayerMask layer) { mask &= ~layer; }
	DLLCLIENT void toggle_layer(VisibilityMask &mask, LayerMask layer) { mask ^= layer; }
	DLLCLIENT LayerMask combine_layers(LayerMask layerA, LayerMask layerB) { return layerA | layerB; }

	enum class LayerFlag : uint8_t {
		CastShadows = 0,
		VisibleInGame,

		Count,
	};
	enum class LayerFlags : uint8_t {
		None = 0,
		CastShadows = 1 << math::to_integral(LayerFlag::CastShadows),
		VisibleInGame = 1 << math::to_integral(LayerFlag::VisibleInGame),

		Default = CastShadows | VisibleInGame,
	};

	class DLLCLIENT LayerManager {
	  public:
		LayerManager();
		void UnregisterLayer(std::string_view name);
		std::optional<LayerMask> RegisterLayer(std::string_view name, LayerFlags flags = LayerFlags::Default);
		LayerMask GetMask(std::string_view name) const;
		bool ShouldPass(LayerMask mask, LayerFlag flag) const;
		VisibilityMask GetFlagMask(LayerFlag flag) const { return m_flagMasks[math::to_integral(flag)]; }
		const string::StringMap<LayerMask> &GetRegisteredLayers() const;
	  private:
		std::array<VisibilityMask, math::to_integral(LayerFlag::Count)> m_flagMasks;
		string::StringMap<LayerMask> m_layers;
		uint8_t m_nextBitIndex = 0;
		std::queue<LayerMask> m_freeMasks;
	};
	using namespace pragma::math::scoped_enum::bitwise;
};
export {
	REGISTER_ENUM_FLAGS(pragma::rendering::LayerFlags)
}
