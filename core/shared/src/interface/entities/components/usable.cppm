// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.usable;

export import :entities.components.base;

export namespace pragma {
	struct DLLNETWORK CEOnUseData : public ComponentEvent {
		CEOnUseData(ecs::BaseEntity *ent);
		virtual void PushArguments(lua::State *l) override;
		ecs::BaseEntity *entity;
	};
	struct DLLNETWORK CECanUseData : public ComponentEvent {
		CECanUseData(ecs::BaseEntity *ent);
		virtual void PushArguments(lua::State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua::State *l) override;
		ecs::BaseEntity *entity;
		bool canUse = true;
	};
	namespace usableComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_USE;
		CLASS_ENUM_COMPAT ComponentEventId EVENT_CAN_USE;
	}
	class DLLNETWORK UsableComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		UsableComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;

		bool CanUse(ecs::BaseEntity *pl) const;
		void OnUse(ecs::BaseEntity *pl);
	};
};
