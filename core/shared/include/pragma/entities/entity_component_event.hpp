// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __ENTITY_COMPONENT_EVENT_HPP__
#define __ENTITY_COMPONENT_EVENT_HPP__

#include "pragma/networkdefinitions.h"

struct lua_State;
namespace pragma {
	class BaseEntityComponent;
	struct DLLNETWORK ComponentEvent {
		virtual void PushArguments(lua_State *l) = 0;
		virtual uint32_t GetReturnCount() { return 0u; }
		virtual void HandleReturnValues(lua_State *l) {}
	};
	struct DLLNETWORK CEGenericComponentEvent : public ComponentEvent {
		virtual void PushArguments(lua_State *l) override;
	};
	struct DLLNETWORK CEOnEntityComponentAdded : public ComponentEvent {
		CEOnEntityComponentAdded(BaseEntityComponent &component);
		BaseEntityComponent &component;
		virtual void PushArguments(lua_State *l) override;
	};
	struct DLLNETWORK CEOnMembersChanged : public ComponentEvent {
		CEOnMembersChanged(BaseEntityComponent &component);
		BaseEntityComponent &component;
		virtual void PushArguments(lua_State *l) override;
	};
	using CEOnEntityComponentRemoved = CEOnEntityComponentAdded;
};

#endif
