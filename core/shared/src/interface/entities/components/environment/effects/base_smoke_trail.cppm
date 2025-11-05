// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.effects.base_smoke_trail;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvSmokeTrailComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		float m_speed = 150.f;
		float m_distance = 400.f;
		float m_minSpriteSize = 45.f;
		float m_maxSpriteSize = 120.f;
		std::string m_material = "particles/smoke_sprites_dense";
	};
};
