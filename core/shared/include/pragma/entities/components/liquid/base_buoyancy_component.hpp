/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_BUOYANCY_COMPONENT_HPP__
#define __BASE_BUOYANCY_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <mathutil/plane.hpp>

namespace pragma
{
	class BaseSurfaceComponent;
	class BaseLiquidControlComponent;
	class BaseLiquidSurfaceSimulationComponent;
	class DLLNETWORK BaseBuoyancyComponent
		: public BaseEntityComponent
	{
	public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
	protected:
		BaseBuoyancyComponent(BaseEntity &ent);
		virtual void OnEndTouch(BaseEntity *ent,PhysObj *phys);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void OnPhysicsInitialized();
		void SimulateBuoyancy() const;
		pragma::ComponentHandle<BaseSurfaceComponent> m_surfaceC {};
		pragma::ComponentHandle<BaseLiquidControlComponent> m_liquidControl {};
		pragma::ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_surfSim {};
	};
};

#endif
