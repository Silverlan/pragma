// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_BUOYANCY_COMPONENT_HPP__
#define __BASE_BUOYANCY_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/plane.hpp>

namespace pragma {
	class BaseSurfaceComponent;
	class BaseLiquidControlComponent;
	class BaseLiquidSurfaceSimulationComponent;
	class DLLNETWORK BaseBuoyancyComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
	  protected:
		BaseBuoyancyComponent(BaseEntity &ent);
		virtual void OnEndTouch(BaseEntity *ent, PhysObj *phys);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void OnPhysicsInitialized();
		void SimulateBuoyancy() const;
		pragma::ComponentHandle<BaseSurfaceComponent> m_surfaceC {};
		pragma::ComponentHandle<BaseLiquidControlComponent> m_liquidControl {};
		pragma::ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_surfSim {};
	};
};

#endif
