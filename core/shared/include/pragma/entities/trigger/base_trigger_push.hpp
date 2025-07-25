// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_TRIGGER_PUSH_HPP__
#define __BASE_TRIGGER_PUSH_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/baseentity_handle.h"
#include <vector>

namespace pragma {
	class BaseToggleComponent;
	class DLLNETWORK BaseTriggerPushComponent : public BaseEntityComponent {
	  public:
		enum class SpawnFlags : uint32_t { ChangeVelocityDirection = 2'048 };
		BaseTriggerPushComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnTick(double dt) override;
	  protected:
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;

		Vector3 m_kvPushDir = {};
		float m_kvPushSpeed = 0.f;
		float m_kvChangeDuration = 0.5f;

		struct PushInfo {
			EntityHandle hEntity;
			Quat rotation = uquat::identity();
			float t = 1.f;
		};
		std::vector<PushInfo> m_entityPushQueue;
		ComponentHandle<pragma::BaseToggleComponent> m_toggleComponent = {};
	};
};

#endif
