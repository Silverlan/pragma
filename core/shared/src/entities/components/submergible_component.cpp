/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/lua/l_entity_handles.hpp"

using namespace pragma;

ComponentEventId SubmergibleComponent::EVENT_ON_WATER_SUBMERGED = INVALID_COMPONENT_ID;
ComponentEventId SubmergibleComponent::EVENT_ON_WATER_EMERGED = INVALID_COMPONENT_ID;
ComponentEventId SubmergibleComponent::EVENT_ON_WATER_ENTERED = INVALID_COMPONENT_ID;
ComponentEventId SubmergibleComponent::EVENT_ON_WATER_EXITED = INVALID_COMPONENT_ID;
void SubmergibleComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_WATER_SUBMERGED = componentManager.RegisterEvent("ON_WATER_SUBMERGED");
	EVENT_ON_WATER_EMERGED = componentManager.RegisterEvent("ON_WATER_EMERGED");
	EVENT_ON_WATER_ENTERED = componentManager.RegisterEvent("ON_WATER_ENTERED");
	EVENT_ON_WATER_EXITED = componentManager.RegisterEvent("ON_WATER_EXITED");
}
SubmergibleComponent::SubmergibleComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_submergedFraction(util::FloatProperty::Create(0.f))
{}
void SubmergibleComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
luabind::object SubmergibleComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SubmergibleComponentHandleWrapper>(l);}
bool SubmergibleComponent::IsSubmerged() const {return (GetSubmergedFraction() >= 0.6f) ? true : false;}
bool SubmergibleComponent::IsFullySubmerged() const {return (GetSubmergedFraction() >= 0.99f) ? true : false;}
bool SubmergibleComponent::IsInWater() const {return (GetSubmergedFraction() > 0.f) ? true : false;}
float SubmergibleComponent::GetSubmergedFraction() const {return *m_submergedFraction;}
void SubmergibleComponent::SetSubmergedFraction(BaseEntity &waterEntity,float fraction)
{
	m_waterEntity = (fraction > 0.f) ? waterEntity.GetHandle() : EntityHandle{};
	auto bInWater = IsInWater();
	auto bSubmerged = IsSubmerged();
	*m_submergedFraction = fraction;

	// Call appropriate callbacks
	if(bInWater == false)
	{
		if(IsInWater() == false)
			return;
		OnWaterEntered();
		if(bSubmerged == false)
		{
			if(IsSubmerged() == false)
				return;
			OnWaterSubmerged();
		}
		return;
	}
	if(IsInWater() == false)
	{
		if(bSubmerged == true && IsSubmerged() == false)
			OnWaterEmerged();
		OnWaterExited();
		return;
	}
	if(bSubmerged == true)
	{
		if(IsSubmerged() == false)
			OnWaterEmerged();
		return;
	}
	else if(IsSubmerged() == true)
		OnWaterSubmerged();
}
void SubmergibleComponent::OnWaterSubmerged() {BroadcastEvent(EVENT_ON_WATER_SUBMERGED);}
void SubmergibleComponent::OnWaterEmerged() {BroadcastEvent(EVENT_ON_WATER_EMERGED);}
void SubmergibleComponent::OnWaterEntered() {BroadcastEvent(EVENT_ON_WATER_ENTERED);}
void SubmergibleComponent::OnWaterExited() {BroadcastEvent(EVENT_ON_WATER_EXITED);}

const util::PFloatProperty &SubmergibleComponent::GetSubmergedFractionProperty() const {return m_submergedFraction;}

BaseEntity *SubmergibleComponent::GetWaterEntity() {return m_waterEntity.get();}
const BaseEntity *SubmergibleComponent::GetWaterEntity() const {return const_cast<SubmergibleComponent*>(this)->GetWaterEntity();}
