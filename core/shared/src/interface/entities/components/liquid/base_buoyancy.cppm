// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.liquid.base_buoyancy;

export import :entities.components.base;
export import :entities.components.base_surface;
export import :entities.components.liquid.base_control;
export import :entities.components.liquid.base_surface_simulation;

export {
	namespace pragma {
		class DLLNETWORK BaseBuoyancyComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;

			virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		  protected:
			BaseBuoyancyComponent(ecs::BaseEntity &ent);
			virtual void OnEndTouch(ecs::BaseEntity *ent, physics::PhysObj *phys);
			virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
			void OnPhysicsInitialized();
			void SimulateBuoyancy() const;
			ComponentHandle<BaseSurfaceComponent> m_surfaceC {};
			ComponentHandle<BaseLiquidControlComponent> m_liquidControl {};
			ComponentHandle<BaseLiquidSurfaceSimulationComponent> m_surfSim {};
		};
	};
};
