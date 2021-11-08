/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

void BaseTimeScaleComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = BaseTimeScaleComponent;

	using TTimeScale = float;
	{
		auto memberInfo = create_component_member_info<
			T,TTimeScale,
			static_cast<void(T::*)(TTimeScale)>(&T::SetTimeScale),
			static_cast<TTimeScale(T::*)() const>(&T::GetTimeScale)
		>("timeScale",1.f);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseTimeScaleComponent::BaseTimeScaleComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_timeScale{util::FloatProperty::Create(1.f)}
{}
void BaseTimeScaleComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"time_scale",false))
			*m_timeScale = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare<std::string>(inputData.input,"settimescale",false))
			*m_timeScale = util::to_float(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_netEvSetTimeScale = SetupNetEvent("set_time_scale");
}
void BaseTimeScaleComponent::SetTimeScale(float timeScale) {*m_timeScale = timeScale;}
float BaseTimeScaleComponent::GetTimeScale() const {return *m_timeScale;}
const util::PFloatProperty &BaseTimeScaleComponent::GetTimeScaleProperty() const {return m_timeScale;}
float BaseTimeScaleComponent::GetEffectiveTimeScale() const {return GetEntity().GetNetworkState()->GetGameState()->GetTimeScale() *GetTimeScale();}
void BaseTimeScaleComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *pAnimatedComponent = dynamic_cast<BaseAnimatedComponent*>(&component);
	if(pAnimatedComponent != nullptr)
	{
		FlagCallbackForRemoval(pAnimatedComponent->GetPlaybackRateProperty()->AddModifier([this](std::reference_wrapper<float> val) {
			val.get() *= GetEffectiveTimeScale();
		}),CallbackType::Component,&component);
	}
}
void BaseTimeScaleComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["timeScale"] = **m_timeScale;
}
void BaseTimeScaleComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	auto timeScale = GetTimeScale();
	udm["timeScale"](timeScale);
	SetTimeScale(timeScale);
}
