/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseColorComponent::EVENT_ON_COLOR_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseColorComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_COLOR_CHANGED = registerEvent("ON_COLOR_CHANGED", ComponentEventInfo::Type::Broadcast); }
void BaseColorComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseColorComponent;
	{
		auto memberInfo = create_component_member_info<T, Vector3, [](const ComponentMemberInfo &, T &component, const Vector3 &value) { component.SetColor(value); }, [](const ComponentMemberInfo &, T &component, Vector3 &value) { value = component.GetColor(); }>("color",
		  Color::White.ToVector3(), AttributeSpecializationType::Color);
		memberInfo.SetInterpolationFunction<T, Vector3, [](const Vector3 &col0, const Vector3 &col1, double t, Vector3 &vOut) {
			double h0, s0, v0;
			util::rgb_to_hsv(col0, h0, s0, v0);

			double h1, s1, v1;
			util::rgb_to_hsv(col1, h1, s1, v1);

			util::lerp_hsv(h0, s0, v0, h1, s1, v1, t);

			vOut = Vector3 {util::hsv_to_rgb(h0, s0, v0)};
		}>();
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, float,
		  [](const ComponentMemberInfo &, T &component, const float &value) {
			  auto col = component.GetColor();
			  col.a = value;
			  component.SetColor(col);
		  },
		  [](const ComponentMemberInfo &, T &component, float &value) { value = component.GetColor().a; }>("alpha", 1.f);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
}
BaseColorComponent::BaseColorComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_color(util::SimpleProperty<util::Vector4Property, Vector4>::Create(Color::White.ToVector4())) {}
BaseColorComponent::~BaseColorComponent()
{
	if(m_cbOnColorChanged.IsValid())
		m_cbOnColorChanged.Remove();
}
void BaseColorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "color", false)) {
			Vector4 r;
			auto n = ustring::string_to_array<glm::vec4::value_type, Double>(kvData.value, &r[0], atof, 4);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				r[i] /= static_cast<float>(std::numeric_limits<uint8_t>::max());
			if(n < 4)
				r.a = 1.f;
			*m_color = r;
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "setcolor", false)) {
			Vector4 r;
			auto n = ustring::string_to_array<glm::vec4::value_type, Double>(inputData.data, &r[0], atof, 4);
			for(auto i = decltype(n) {0u}; i < n; ++i)
				r[i] /= static_cast<float>(std::numeric_limits<uint8_t>::max());
			if(n < 4)
				r.a = 1.f;
			*m_color = r;
		}
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnColorChanged = m_color->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> newColor) {
		pragma::CEOnColorChanged onColorChanged {oldColor.get(), newColor.get()};
		BroadcastEvent(EVENT_ON_COLOR_CHANGED, onColorChanged);
	});

	auto &ent = GetEntity();
	//ent.AddComponent("io");
	m_netEvSetColor = SetupNetEvent("set_color");
}
void BaseColorComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["color"] = m_color->GetValue();
}
void BaseColorComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	Vector4 color;
	udm["color"](color);
	(*m_color) = color;
}
const Vector4 &BaseColorComponent::GetColor() const { return *m_color; }
const util::PVector4Property &BaseColorComponent::GetColorProperty() const { return m_color; }

void BaseColorComponent::SetColor(const Color &color) { *m_color = color.ToVector4(); }
void BaseColorComponent::SetColor(const Vector4 &color) { *m_color = color; }
void BaseColorComponent::SetColor(const Vector3 &color) { *m_color = Vector4 {color, (*m_color)->a}; }

/////////////////

CEOnColorChanged::CEOnColorChanged(const Vector4 &oldColor, const Vector4 &color) : oldColor {oldColor}, color {color} {}
void CEOnColorChanged::PushArguments(lua_State *l)
{
	Lua::Push<Vector4>(l, oldColor);
	Lua::Push<Vector4>(l, color);
}
