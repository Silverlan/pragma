// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.liquid.base_buoyancy;

export import :entities.components.base;
export import :entities.components.base_surface;
export import :entities.components.liquid.base_control;
export import :entities.components.liquid.base_surface_simulation;

export {
	class PhysObj;
	namespace pragma {
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
};
