// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.effects.base_sprite;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvSpriteComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		enum class DLLNETWORK SpawnFlags : uint32_t { BlackToAlpha = 4'096, NoSoftParticles = BlackToAlpha << 1 };
		uint32_t m_particleRenderMode = std::numeric_limits<uint32_t>::max();
		std::string m_spritePath;
		float m_size = 1.f;
		float m_bloomScale = 0.f;
		float m_tFadeIn = 0.f;
		float m_tFadeOut = 0.f;
		Color m_color = {};
	};
};
