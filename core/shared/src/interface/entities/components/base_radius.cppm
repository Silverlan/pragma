// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_radius;

export import :entities.components.base;

export namespace pragma {
	namespace baseRadiusComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_RADIUS_CHANGED)
	}
	class DLLNETWORK BaseRadiusComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		float GetRadius() const;
		const util::PFloatProperty &GetRadiusProperty() const;
		void SetRadius(float radius);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseRadiusComponent(ecs::BaseEntity &ent);
		NetEventId m_netEvSetRadius = INVALID_NET_EVENT;
		util::PFloatProperty m_radius;
	};
	struct DLLNETWORK CEOnRadiusChanged : public ComponentEvent {
		CEOnRadiusChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua::State *l) override;
		float oldRadius;
		float newRadius;
	};
};
