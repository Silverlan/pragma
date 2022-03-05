/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/util.h>
#include "pragma/util/util_handled.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <algorithm>
#include <udm.hpp>

using namespace pragma;

void BaseEnvLightSpotComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = BaseEnvLightSpotComponent;

	{
		using TBlend = float;
		auto memberInfo = create_component_member_info<
			T,TBlend,
			static_cast<void(T::*)(TBlend)>(&T::SetBlendFraction),
			static_cast<TBlend(T::*)() const>(&T::GetBlendFraction)
		>("blendFraction",0.1f,AttributeSpecializationType::Fraction);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(1.f);
		registerMember(std::move(memberInfo));
	}
	
	using TConeAngle = float;
	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetOuterConeAngle),
			static_cast<TConeAngle(T::*)() const>(&T::GetOuterConeAngle)
		>("outerConeAngle",0.f,AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(179.99f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetConeStartOffset),
			static_cast<TConeAngle(T::*)() const>(&T::GetConeStartOffset)
		>("coneStartOffset",0.f,AttributeSpecializationType::Distance);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseEnvLightSpotComponent::BaseEnvLightSpotComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_blendFraction(util::FloatProperty::Create(0.1f)),
	m_outerConeAngle(util::FloatProperty::Create(0.f)),m_coneStartOffset(util::FloatProperty::Create(0.f))
{}
void BaseEnvLightSpotComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"outerconeangle",false))
			*m_outerConeAngle = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"outercutoff",false))
			*m_outerConeAngle = util::to_float(kvData.value) *2.f;
		else if(ustring::compare<std::string>(kvData.key,"blendfraction",false))
			*m_blendFraction = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"cone_start_offset",false))
			SetConeStartOffset(util::to_float(kvData.value));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("light");
	ent.AddComponent("radius");
	ent.AddComponent("point_at_target");
	m_netEvSetConeStartOffset = SetupNetEvent("set_cone_start_offset");
}

void BaseEnvLightSpotComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["blendFraction"] = **m_blendFraction;
	udm["outerConeAngle"] = **m_outerConeAngle;
	udm["coneStartOffset"] = **m_coneStartOffset;
}
void BaseEnvLightSpotComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	udm["blendFraction"](**m_blendFraction);
	udm["outerConeAngle"](**m_outerConeAngle);
	udm["coneStartOffset"](**m_coneStartOffset);
}

void BaseEnvLightSpotComponent::SetOuterConeAngle(umath::Degree ang) {*m_outerConeAngle = ang;}
umath::Degree BaseEnvLightSpotComponent::GetOuterConeAngle() const {return *m_outerConeAngle;}
umath::Fraction BaseEnvLightSpotComponent::GetBlendFraction() const {return *m_blendFraction;}
void BaseEnvLightSpotComponent::SetBlendFraction(umath::Fraction fraction) {*m_blendFraction = fraction;}

void BaseEnvLightSpotComponent::SetConeStartOffset(float offset) {*m_coneStartOffset = offset;}
float BaseEnvLightSpotComponent::GetConeStartOffset() const {return *m_coneStartOffset;}
