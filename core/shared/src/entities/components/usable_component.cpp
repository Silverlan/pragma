/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"

using namespace pragma;

pragma::ComponentEventId UsableComponent::EVENT_ON_USE = pragma::INVALID_COMPONENT_ID;
pragma::ComponentEventId UsableComponent::EVENT_CAN_USE = pragma::INVALID_COMPONENT_ID;
void UsableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_USE = registerEvent("ON_USE", ComponentEventInfo::Type::Broadcast);
	EVENT_CAN_USE = registerEvent("CAN_USE", ComponentEventInfo::Type::Broadcast);
}
UsableComponent::UsableComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void UsableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("transform");
}
void UsableComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool UsableComponent::CanUse(BaseEntity *ent) const
{
	pragma::CECanUseData evCanUse {ent};
	BroadcastEvent(EVENT_CAN_USE, evCanUse);
	return evCanUse.canUse;
}
void UsableComponent::OnUse(BaseEntity *ent)
{
	pragma::CEOnUseData evData {ent};
	BroadcastEvent(EVENT_ON_USE, evData);
}

////////

CEOnUseData::CEOnUseData(BaseEntity *ent) : entity(ent) {}
void CEOnUseData::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

////////

CECanUseData::CECanUseData(BaseEntity *ent) : entity(ent) {}
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
