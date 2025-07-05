// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENV_SMOKE_TRAIL_H__
#define __ENV_SMOKE_TRAIL_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
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

#endif
