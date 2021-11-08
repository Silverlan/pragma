/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseRadiusComponent::EVENT_ON_RADIUS_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseRadiusComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
	EVENT_ON_RADIUS_CHANGED = registerEvent("ON_RADIUS_CHANGED",EntityComponentManager::EventInfo::Type::Broadcast);
}
void BaseRadiusComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = BaseRadiusComponent;
	using TRadius = float;
	{
		auto memberInfo = create_component_member_info<
			T,TRadius,
			static_cast<void(T::*)(TRadius)>(&T::SetRadius),
			static_cast<TRadius(T::*)() const>(&T::GetRadius)
		>("radius",0.f,AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseRadiusComponent::BaseRadiusComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_radius(util::FloatProperty::Create(0.f))
{}
void BaseRadiusComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"radius",false))
			SetRadius(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare<std::string>(inputData.input,"setradius",false))
			SetRadius(util::to_float(inputData.data));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	m_netEvSetRadius = SetupNetEvent("set_radius");
}
void BaseRadiusComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["radius"] = **m_radius;
}
void BaseRadiusComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	auto radius = GetRadius();
	udm["radius"](radius);
	SetRadius(radius);
}
float BaseRadiusComponent::GetRadius() const {return *m_radius;}
const util::PFloatProperty &BaseRadiusComponent::GetRadiusProperty() const {return m_radius;}
void BaseRadiusComponent::SetRadius(float radius)
{
	auto oldRadius = GetRadius();
	*m_radius = radius;

	CEOnRadiusChanged evData {oldRadius,*m_radius};
	BroadcastEvent(EVENT_ON_RADIUS_CHANGED,evData);
}

//////////////

CEOnRadiusChanged::CEOnRadiusChanged(float oldRadius,float newRadius)
	: oldRadius{oldRadius},newRadius{newRadius}
{}
void CEOnRadiusChanged::PushArguments(lua_State *l)
{
	Lua::PushNumber(l,oldRadius);
	Lua::PushNumber(l,newRadius);
}
