// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.logic_component;

export import :entities.components.base;

export namespace pragma {
	namespace logicComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_TICK)
	}
	class DLLNETWORK LogicComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		LogicComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void OnTick(double tDelta) override;

		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
	};
	struct DLLNETWORK CEOnTick : public ComponentEvent {
		CEOnTick(double dt);
		virtual void PushArguments(lua::State *l) override;
		double deltaTime;
	};
};
