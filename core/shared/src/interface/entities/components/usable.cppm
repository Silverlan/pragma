// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.usable;

export import :entities.components.base;

export namespace pragma {
	struct DLLNETWORK CEOnUseData : public ComponentEvent {
		CEOnUseData(pragma::ecs::BaseEntity *ent);
		virtual void PushArguments(lua::State *l) override;
		pragma::ecs::BaseEntity *entity;
	};
	struct DLLNETWORK CECanUseData : public ComponentEvent {
		CECanUseData(pragma::ecs::BaseEntity *ent);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		pragma::ecs::BaseEntity *entity;
		bool canUse = true;
	};
	namespace usableComponent {
		CLASS_ENUM_COMPAT pragma::ComponentEventId EVENT_ON_USE;
		CLASS_ENUM_COMPAT pragma::ComponentEventId EVENT_CAN_USE;
	}
	class DLLNETWORK UsableComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		UsableComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		bool CanUse(pragma::ecs::BaseEntity *pl) const;
		void OnUse(pragma::ecs::BaseEntity *pl);
	};
};
