/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENV_TIMESCALE_H__
#define __ENV_TIMESCALE_H__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	class DLLNETWORK BaseEnvTimescaleComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		float m_kvTimescale = 1.f;
		float m_kvInnerRadius = 0.f;
		float m_kvOuterRadius = 0.f;
	};
};

#endif
