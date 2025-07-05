// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
