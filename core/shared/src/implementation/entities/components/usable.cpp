// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cinttypes>
#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :entities.components.usable;

using namespace pragma;

pragma::ComponentEventId UsableComponent::EVENT_ON_USE = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId UsableComponent::EVENT_CAN_USE = pragma::INVALID_COMPONENT_ID;
void UsableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_USE = registerEvent("ON_USE", ComponentEventInfo::Type::Broadcast);
	EVENT_CAN_USE = registerEvent("CAN_USE", ComponentEventInfo::Type::Broadcast);
}
UsableComponent::UsableComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void UsableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("transform");
}
void UsableComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool UsableComponent::CanUse(pragma::ecs::BaseEntity *ent) const
{
	pragma::CECanUseData evCanUse {ent};
	BroadcastEvent(EVENT_CAN_USE, evCanUse);
	return evCanUse.canUse;
}
void UsableComponent::OnUse(pragma::ecs::BaseEntity *ent)
{
	pragma::CEOnUseData evData {ent};
	BroadcastEvent(EVENT_ON_USE, evData);
}

////////

CEOnUseData::CEOnUseData(pragma::ecs::BaseEntity *ent) : entity(ent) {}
void CEOnUseData::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

////////

CECanUseData::CECanUseData(pragma::ecs::BaseEntity *ent) : entity(ent) {}
void CECanUseData::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}
uint32_t CECanUseData::GetReturnCount() { return 1u; }
void CECanUseData::HandleReturnValues(lua_State *l)
{
	if(Lua::IsBool(l, -1))
		canUse = Lua::CheckBool(l, -1);
}
