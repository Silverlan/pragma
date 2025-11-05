// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.base_time_scale;

export import :entities.components.base;

export namespace pragma {
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
