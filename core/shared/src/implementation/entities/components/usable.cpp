// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.usable;

using namespace pragma;

void UsableComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	usableComponent::EVENT_ON_USE = registerEvent("ON_USE", ComponentEventInfo::Type::Broadcast);
	usableComponent::EVENT_CAN_USE = registerEvent("CAN_USE", ComponentEventInfo::Type::Broadcast);
}
UsableComponent::UsableComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void UsableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("transform");
}
void UsableComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

bool UsableComponent::CanUse(ecs::BaseEntity *ent) const
{
	CECanUseData evCanUse {ent};
	BroadcastEvent(usableComponent::EVENT_CAN_USE, evCanUse);
	return evCanUse.canUse;
}
void UsableComponent::OnUse(ecs::BaseEntity *ent)
{
	CEOnUseData evData {ent};
	BroadcastEvent(usableComponent::EVENT_ON_USE, evData);
}

////////

CEOnUseData::CEOnUseData(ecs::BaseEntity *ent) : entity(ent) {}
void CEOnUseData::PushArguments(lua::State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

////////

CECanUseData::CECanUseData(ecs::BaseEntity *ent) : entity(ent) {}
void CECanUseData::PushArguments(lua::State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}
uint32_t CECanUseData::GetReturnCount() { return 1u; }
void CECanUseData::HandleReturnValues(lua::State *l)
{
	if(Lua::IsBool(l, -1))
		canUse = Lua::CheckBool(l, -1);
}
