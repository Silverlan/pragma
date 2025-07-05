// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __USABLE_COMPONENT_HPP__
#define __USABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma {
	struct DLLNETWORK CEOnUseData : public ComponentEvent {
		CEOnUseData(BaseEntity *ent);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *entity;
	};
	struct DLLNETWORK CECanUseData : public ComponentEvent {
		CECanUseData(BaseEntity *ent);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		BaseEntity *entity;
		bool canUse = true;
	};
	class DLLNETWORK UsableComponent final : public BaseEntityComponent {
	  public:
		static pragma::ComponentEventId EVENT_ON_USE;
		static pragma::ComponentEventId EVENT_CAN_USE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		UsableComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;

		bool CanUse(BaseEntity *pl) const;
		void OnUse(BaseEntity *pl);
	};
};

#endif
