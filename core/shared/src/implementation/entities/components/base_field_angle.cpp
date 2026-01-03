// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_field_angle;

using namespace pragma;

void BaseFieldAngleComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED = registerEvent("ON_FIELD_ANGLE_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseFieldAngleComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
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
BaseFieldAngleComponent::BaseFieldAngleComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_fieldAngle(util::FloatProperty::Create(0.f)) {}
void BaseFieldAngleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "coneAngle", false))
			SetFieldAngle(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setconeangle", false))
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
math::Degree BaseFieldAngleComponent::GetFieldAngle() const { return *m_fieldAngle; }
const util::PFloatProperty &BaseFieldAngleComponent::GetFieldAngleProperty() const { return m_fieldAngle; }
void BaseFieldAngleComponent::SetFieldAngle(math::Degree coneAngle)
{
	auto oldFieldAngle = GetFieldAngle();
	*m_fieldAngle = coneAngle;

	CEOnFieldAngleChanged evData {oldFieldAngle, *m_fieldAngle};
	BroadcastEvent(baseFieldAngleComponent::EVENT_ON_FIELD_ANGLE_CHANGED, evData);
}

//////////////

CEOnFieldAngleChanged::CEOnFieldAngleChanged(float oldFieldAngle, float newFieldAngle) : oldFieldAngle {oldFieldAngle}, newFieldAngle {newFieldAngle} {}
void CEOnFieldAngleChanged::PushArguments(lua::State *l)
{
	Lua::PushNumber(l, oldFieldAngle);
	Lua::PushNumber(l, newFieldAngle);
}
