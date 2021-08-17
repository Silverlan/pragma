/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/animated_2_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/model/animation/animation_channel.hpp"
#include <udm.hpp>

using namespace pragma;
#pragma optimize("",off)
ComponentEventId BaseColorComponent::EVENT_ON_COLOR_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseColorComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_COLOR_CHANGED = componentManager.RegisterEvent("ON_COLOR_CHANGED");
}
void BaseColorComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,const std::function<ComponentMemberIndex(ComponentMemberInfo&&)> &registerMember)
{
	using T = BaseColorComponent;
	auto memberInfo = create_component_member_info<
		T,Vector4,
		static_cast<void(BaseColorComponent::*)(const Vector4&)>(&BaseColorComponent::SetColor),
		[](const ComponentMemberInfo&,T &component,Vector4 &value) {value = component.GetColor().ToVector4();}
	>("color");
	memberInfo.SetInterpolationFunction<T,Vector4,[](const Vector4 &col0,const Vector4 &col1,double t,Vector4 &vOut) {
		double h0,s0,v0;
		util::rgb_to_hsv(col0,h0,s0,v0);

		double h1,s1,v1;
		util::rgb_to_hsv(col1,h1,s1,v1);

		util::lerp_hsv(h0,s0,v0,h1,s1,v1,t);

		vOut = Vector4{
			util::hsv_to_rgb(h0,s0,v0),
			umath::lerp(col0.a,col1.a,t)
		};
	}>();
	registerMember(std::move(memberInfo));
}
BaseColorComponent::BaseColorComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_color(util::SimpleProperty<util::ColorProperty,Color>::Create(Color(255,255,255,255)))
{}
BaseColorComponent::~BaseColorComponent()
{
	if(m_cbOnColorChanged.IsValid())
		m_cbOnColorChanged.Remove();
}
void BaseColorComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"color",false))
			*m_color = Color{kvData.value};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setcolor",false))
			*m_color = Color{inputData.data};
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	m_cbOnColorChanged = m_color->AddCallback([this](std::reference_wrapper<const Color> oldColor,std::reference_wrapper<const Color> newColor) {
		pragma::CEOnColorChanged onColorChanged{oldColor.get(),newColor.get()};
		BroadcastEvent(EVENT_ON_COLOR_CHANGED,onColorChanged);
	});

	auto &ent = GetEntity();
	//ent.AddComponent("io");
	m_netEvSetColor = SetupNetEvent("set_color");
}
void BaseColorComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["color"] = (*m_color)->ToVector4();
}
void BaseColorComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	Vector4 color;
	udm["color"](color);
	(*m_color) = color;
}
const Color &BaseColorComponent::GetColor() const {return *m_color;}
const util::PColorProperty &BaseColorComponent::GetColorProperty() const {return m_color;}

void BaseColorComponent::SetColor(const Color &color) {*m_color = color;}
void BaseColorComponent::SetColor(const Vector4 &color) {SetColor(Color(color));}
void BaseColorComponent::SetColor(const Vector3 &color) {SetColor(Vector4{color.x,color.y,color.z,GetColor().ToVector4().w});}

/////////////////

CEOnColorChanged::CEOnColorChanged(const Color &oldColor,const Color &color)
	: oldColor{oldColor},color{color}
{}
void CEOnColorChanged::PushArguments(lua_State *l)
{
	Lua::Push<Color>(l,oldColor);
	Lua::Push<Color>(l,color);
}
#pragma optimize("",on)
