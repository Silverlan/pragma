/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_field_angle_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseFieldAngleComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_FIELD_ANGLE_CHANGED = registerEvent("ON_FIELD_ANGLE_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseFieldAngleComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseFieldAngleComponent;
	using TRadius = float;
	{
		auto memberInfo = create_component_member_info<T, TRadius, static_cast<void (T::*)(TRadius)>(&T::SetFieldAngle), static_cast<TRadius (T::*)() const>(&T::GetFieldAngle)>("coneAngle", 0.f, AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(180.f);
		registerMember(std::move(memberInfo));
	}
}
BaseFieldAngleComponent::BaseFieldAngleComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_fieldAngle(util::FloatProperty::Create(0.f)) {}
void BaseFieldAngleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "coneAngle", false))
			SetFieldAngle(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setconeangle", false))
			SetFieldAngle(util::to_float(inputData.data));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	m_netEvSetFieldAngle = SetupNetEvent("set_cone_angle");
}
void BaseFieldAngleComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["fieldAngle"] = **m_fieldAngle;
}
void BaseFieldAngleComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto fieldAngle = GetFieldAngle();
	udm["fieldAngle"](fieldAngle);
	SetFieldAngle(fieldAngle);
}
umath::Degree BaseFieldAngleComponent::GetFieldAngle() const { return *m_fieldAngle; }
const util::PFloatProperty &BaseFieldAngleComponent::GetFieldAngleProperty() const { return m_fieldAngle; }
void BaseFieldAngleComponent::SetFieldAngle(umath::Degree coneAngle)
{
	auto oldFieldAngle = GetFieldAngle();
	*m_fieldAngle = coneAngle;

	CEOnFieldAngleChanged evData {oldFieldAngle, *m_fieldAngle};
	BroadcastEvent(EVENT_ON_FIELD_ANGLE_CHANGED, evData);
}

//////////////

CEOnFieldAngleChanged::CEOnFieldAngleChanged(float oldFieldAngle, float newFieldAngle) : oldFieldAngle {oldFieldAngle}, newFieldAngle {newFieldAngle} {}
void CEOnFieldAngleChanged::PushArguments(lua_State *l)
{
	Lua::PushNumber(l, oldFieldAngle);
	Lua::PushNumber(l, newFieldAngle);
}
