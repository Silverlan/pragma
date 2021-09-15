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

void BaseEnvLightSpotComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,const std::function<ComponentMemberIndex(ComponentMemberInfo&&)> &registerMember)
{
	using T = BaseEnvLightSpotComponent;

	using TConeAngle = float;
	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetInnerCutoffAngle),
			static_cast<TConeAngle(T::*)() const>(&T::GetInnerCutoffAngle)
		>("innerConeAngle",0.f,AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(179.99f);
		memberInfo.updateDependenciesFunction = [](BaseEntityComponent &component,std::vector<std::string> &outAffectedProps) {
			auto &c = static_cast<T&>(component);
			auto outerAngle = c.GetOuterCutoffAngle();
			auto innerAngle = c.GetInnerCutoffAngle();
			if(innerAngle >= outerAngle)
			{
				c.SetOuterCutoffAngle(innerAngle +0.01f);
				outAffectedProps.push_back("outerConeAngle");
			}
		};
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<
			T,TConeAngle,
			static_cast<void(T::*)(TConeAngle)>(&T::SetOuterCutoffAngle),
			static_cast<TConeAngle(T::*)() const>(&T::GetOuterCutoffAngle)
		>("outerConeAngle",0.f,AttributeSpecializationType::Angle);
		memberInfo.SetMin(0.f);
		memberInfo.SetMax(179.99f);
		memberInfo.updateDependenciesFunction = [](BaseEntityComponent &component,std::vector<std::string> &outAffectedProps) {
			auto &c = static_cast<T&>(component);
			auto outerAngle = c.GetOuterCutoffAngle();
			auto innerAngle = c.GetInnerCutoffAngle();
			if(outerAngle < innerAngle)
			{
				c.SetInnerCutoffAngle(outerAngle -0.01f);
				outAffectedProps.push_back("innerConeAngle");
			}
		};
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
	: BaseEntityComponent(ent),m_angInnerCutoff(util::FloatProperty::Create(0.f)),
	m_angOuterCutoff(util::FloatProperty::Create(0.f)),m_coneStartOffset(util::FloatProperty::Create(0.f))
{}
void BaseEnvLightSpotComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare<std::string>(kvData.key,"outercutoff",false))
			*m_angOuterCutoff = util::to_float(kvData.value);
		else if(ustring::compare<std::string>(kvData.key,"innercutoff",false))
			*m_angInnerCutoff = util::to_float(kvData.value);
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
	udm["innerConeAngle"] = **m_angInnerCutoff;
	udm["outerConeAngle"] = **m_angOuterCutoff;
	udm["coneStartOffset"] = **m_coneStartOffset;
}
void BaseEnvLightSpotComponent::Load(udm::LinkedPropertyWrapperArg udm,uint32_t version)
{
	BaseEntityComponent::Load(udm,version);
	udm["innerConeAngle"](**m_angInnerCutoff);
	udm["outerConeAngle"](**m_angOuterCutoff);
	udm["coneStartOffset"](**m_coneStartOffset);
}

void BaseEnvLightSpotComponent::SetOuterCutoffAngle(umath::Degree ang) {*m_angOuterCutoff = ang;}
umath::Degree BaseEnvLightSpotComponent::GetOuterCutoffAngle() const {return *m_angOuterCutoff;}
void BaseEnvLightSpotComponent::SetInnerCutoffAngle(umath::Degree ang) {*m_angInnerCutoff = ang;}
umath::Degree BaseEnvLightSpotComponent::GetInnerCutoffAngle() const {return *m_angInnerCutoff;}

void BaseEnvLightSpotComponent::SetConeStartOffset(float offset) {*m_coneStartOffset = offset;}
float BaseEnvLightSpotComponent::GetConeStartOffset() const {return *m_coneStartOffset;}
