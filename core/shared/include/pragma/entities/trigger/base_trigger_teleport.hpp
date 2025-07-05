// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_TRIGGER_TELEPORT_HPP__
#define __BASE_TRIGGER_TELEPORT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include <vector>

namespace pragma {
	class DLLNETWORK BaseTriggerTeleportComponent : public BaseEntityComponent {
	  public:
		using BaseEntityComponent::BaseEntityComponent;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
	  protected:
		std::string m_target;
		enum class SpawnFlags : uint32_t { FaceTargetDirectionOnTeleport = 512 };
	};
};

#endif
