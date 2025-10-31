// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.logic_component;

export import :entities.components.base;

export namespace pragma {
	class DLLNETWORK LogicComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_TICK;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		LogicComponent(pragma::ecs::BaseEntity &ent);
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
