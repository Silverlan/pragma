// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_radius;

export import :entities.components.base;

export namespace pragma {
	namespace baseRadiusComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_RADIUS_CHANGED;
	}
	class DLLNETWORK BaseRadiusComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		float GetRadius() const;
		const pragma::util::PFloatProperty &GetRadiusProperty() const;
		void SetRadius(float radius);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseRadiusComponent(pragma::ecs::BaseEntity &ent);
		pragma::NetEventId m_netEvSetRadius = pragma::INVALID_NET_EVENT;
		pragma::util::PFloatProperty m_radius;
	};
	struct DLLNETWORK CEOnRadiusChanged : public ComponentEvent {
		CEOnRadiusChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua::State *l) override;
		float oldRadius;
		float newRadius;
	};
};
