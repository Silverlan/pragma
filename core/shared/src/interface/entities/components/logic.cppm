// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/entities/components/base_entity_component.hpp"

export module pragma.shared:entities.components.logic;

export namespace pragma {
	class DLLNETWORK LogicComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_TICK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		LogicComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void OnTick(double tDelta) override;

		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	};
	struct DLLNETWORK CEOnTick : public ComponentEvent {
		CEOnTick(double dt);
		virtual void PushArguments(lua_State *l) override;
		double deltaTime;
	};
};
