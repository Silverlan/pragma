// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LOGIC_RELAY_H__
#define __LOGIC_RELAY_H__

#include "pragma/entities/components/base_entity_component.hpp"
#include <string>

namespace pragma {
	class DLLNETWORK BaseLogicRelayComponent : public pragma::BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
	};
};

#endif
