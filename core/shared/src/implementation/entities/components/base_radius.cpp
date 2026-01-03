// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_radius;

using namespace pragma;

void BaseRadiusComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { baseRadiusComponent::EVENT_ON_RADIUS_CHANGED = registerEvent("ON_RADIUS_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseRadiusComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseRadiusComponent;
	using TRadius = float;
	{
		auto memberInfo = create_component_member_info<T, TRadius, static_cast<void (T::*)(TRadius)>(&T::SetRadius), static_cast<TRadius (T::*)() const>(&T::GetRadius)>("radius", 0.f, AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseRadiusComponent::BaseRadiusComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_radius(util::FloatProperty::Create(0.f)) {}
void BaseRadiusComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "radius", false))
			SetRadius(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "setradius", false))
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
void BaseRadiusComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	auto radius = GetRadius();
	udm["radius"](radius);
	SetRadius(radius);
}
float BaseRadiusComponent::GetRadius() const { return *m_radius; }
const util::PFloatProperty &BaseRadiusComponent::GetRadiusProperty() const { return m_radius; }
void BaseRadiusComponent::SetRadius(float radius)
{
	auto oldRadius = GetRadius();
	*m_radius = radius;

	CEOnRadiusChanged evData {oldRadius, *m_radius};
	BroadcastEvent(baseRadiusComponent::EVENT_ON_RADIUS_CHANGED, evData);
}

//////////////

CEOnRadiusChanged::CEOnRadiusChanged(float oldRadius, float newRadius) : oldRadius {oldRadius}, newRadius {newRadius} {}
void CEOnRadiusChanged::PushArguments(lua::State *l)
{
	Lua::PushNumber(l, oldRadius);
	Lua::PushNumber(l, newRadius);
}
