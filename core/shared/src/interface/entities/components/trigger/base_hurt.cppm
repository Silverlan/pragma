// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <vector>

export module pragma.shared:entities.components.triggers.base_hurt;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK BaseTriggerHurtComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};
