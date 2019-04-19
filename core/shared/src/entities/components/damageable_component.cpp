#include "stdafx_shared.h"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"

using namespace pragma;

ComponentEventId DamageableComponent::EVENT_ON_TAKE_DAMAGE = pragma::INVALID_COMPONENT_ID;
void DamageableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_TAKE_DAMAGE = componentManager.RegisterEvent("ON_TAKE_DAMAGE");
}
DamageableComponent::DamageableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void DamageableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("health");
}

luabind::object DamageableComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<DamageableComponentHandleWrapper>(l);}

void DamageableComponent::OnTakeDamage(DamageInfo &info) {}

void DamageableComponent::TakeDamage(DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	game->CallCallbacks<void,BaseEntity*,std::reference_wrapper<DamageInfo>>("OnEntityTakeDamage",&ent,std::ref<DamageInfo>(info));
	OnTakeDamage(info);

	CEOnTakeDamage takeDmgInfo {info};
	BroadcastEvent(EVENT_ON_TAKE_DAMAGE,takeDmgInfo);
}

//////////////

CEOnTakeDamage::CEOnTakeDamage(DamageInfo &damageInfo)
	: damageInfo{damageInfo}
{}
void CEOnTakeDamage::PushArguments(lua_State *l)
{
	Lua::Push<DamageInfo*>(l,&damageInfo);
}
