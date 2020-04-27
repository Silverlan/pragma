/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __USABLE_COMPONENT_HPP__
#define __USABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	struct DLLNETWORK CEOnUseData
		: public ComponentEvent
	{
		CEOnUseData(BaseEntity *ent);
		virtual void PushArguments(lua_State *l) override;
		BaseEntity *entity;
	};
	struct DLLNETWORK CECanUseData
		: public ComponentEvent
	{
		CECanUseData(BaseEntity *ent);
		virtual void PushArguments(lua_State *l) override;
		virtual uint32_t GetReturnCount() override;
		virtual void HandleReturnValues(lua_State *l) override;
		BaseEntity *entity;
		bool canUse = true;
	};
	class DLLNETWORK UsableComponent final
		: public BaseEntityComponent
	{
	public:
		static pragma::ComponentEventId EVENT_ON_USE;
		static pragma::ComponentEventId EVENT_CAN_USE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		UsableComponent(BaseEntity &ent);
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		bool CanUse(BaseEntity *pl) const;
		void OnUse(BaseEntity *pl);
	};
};

#endif
