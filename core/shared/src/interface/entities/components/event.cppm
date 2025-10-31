// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.shared:entities.components.events.event;

export namespace pragma {
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
