// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.environment.effects.base_fire;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseEnvFireComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	  protected:
		std::string m_fireType = "fire";
	};
};
