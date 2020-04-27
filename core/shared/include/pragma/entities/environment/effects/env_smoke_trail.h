/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __ENV_SMOKE_TRAIL_H__
#define __ENV_SMOKE_TRAIL_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	class DLLNETWORK BaseEnvSmokeTrailComponent
		: public BaseEntityComponent
	{
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
