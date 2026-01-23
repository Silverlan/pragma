// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.triggers.base_teleport;

export import :entities.components.base;

export namespace pragma {
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
